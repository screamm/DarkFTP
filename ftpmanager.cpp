// ftpmanager.cpp
#include "ftpmanager.h"

#include <QUrl>
#include <QDateTime>
#include <QRegularExpression>
#include <QFileInfo>
#include <QDir>

FtpManager::FtpManager(QObject *parent)
    : QObject(parent),
    m_manager(new QNetworkAccessManager(this)),
    m_port(21),
    m_connected(false),
    m_currentDirectory("/"),
    m_currentListReply(nullptr),
    m_currentUploadReply(nullptr),
    m_currentDownloadReply(nullptr)
{
    connect(m_manager, &QNetworkAccessManager::authenticationRequired,
            this, &FtpManager::onAuthenticationRequired);
}

FtpManager::~FtpManager()
{
    disconnectFromHost();
}

void FtpManager::connectToHost(const QString &host, const QString &username, 
                              const QString &password, quint16 port)
{
    // Spara anslutningsinformationen
    m_host = host;
    m_username = username;
    m_password = password;
    m_port = port;
    
    // Lista roten för att testa anslutningen
    listDirectory("/");
}

void FtpManager::disconnectFromHost()
{
    if (m_connected) {
        // Avbryt pågående överföringar
        if (m_currentListReply) {
            m_currentListReply->abort();
            m_currentListReply->deleteLater();
            m_currentListReply = nullptr;
        }
        
        if (m_currentUploadReply) {
            m_currentUploadReply->abort();
            m_currentUploadReply->deleteLater();
            m_currentUploadReply = nullptr;
        }
        
        if (m_currentDownloadReply) {
            m_currentDownloadReply->abort();
            m_currentDownloadReply->deleteLater();
            m_currentDownloadReply = nullptr;
        }
        
        m_connected = false;
        emit disconnected();
    }
}

bool FtpManager::isConnected() const
{
    return m_connected;
}

QString FtpManager::currentDirectory() const
{
    return m_currentDirectory;
}

void FtpManager::listDirectory(const QString &path)
{
    QString dirPath = path;
    if (dirPath.isEmpty()) {
        dirPath = m_currentDirectory;
    }
    
    QUrl url = createUrl(dirPath);
    
    QNetworkRequest request(url);
    m_currentListReply = m_manager->get(request);
    
    connect(m_currentListReply, &QNetworkReply::finished, this, &FtpManager::onListFinished);
    connect(m_currentListReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &FtpManager::onNetworkError);
}

void FtpManager::uploadFile(const QString &localFilePath, const QString &remoteFilePath)
{
    QFile *file = new QFile(localFilePath, this);
    if (!file->open(QIODevice::ReadOnly)) {
        emit error(tr("Kunde inte öppna lokal fil: %1").arg(file->errorString()));
        file->deleteLater();
        return;
    }
    
    QUrl url = createUrl(remoteFilePath);
    QNetworkRequest request(url);
    
    m_currentLocalUploadPath = localFilePath;
    m_currentUploadPath = remoteFilePath;
    
    m_currentUploadReply = m_manager->put(request, file);
    
    connect(m_currentUploadReply, &QNetworkReply::uploadProgress, 
            this, &FtpManager::onUploadProgress);
            
    connect(m_currentUploadReply, &QNetworkReply::finished,
            this, &FtpManager::onUploadFinished);
            
    connect(m_currentUploadReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &FtpManager::onNetworkError);
    
    // Filobjektet raderas automatiskt när svaret är klart
    file->setParent(m_currentUploadReply);
}

void FtpManager::downloadFile(const QString &remoteFilePath, const QString &localFilePath)
{
    QUrl url = createUrl(remoteFilePath);
    QNetworkRequest request(url);
    
    m_currentDownloadPath = remoteFilePath;
    m_currentLocalDownloadPath = localFilePath;
    
    // Skapa katalogstruktur om den inte finns
    QFileInfo fileInfo(localFilePath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    m_currentDownloadReply = m_manager->get(request);
    
    connect(m_currentDownloadReply, &QNetworkReply::downloadProgress,
            this, &FtpManager::onDownloadProgress);
            
    connect(m_currentDownloadReply, &QNetworkReply::finished,
            this, &FtpManager::onDownloadFinished);
            
    connect(m_currentDownloadReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &FtpManager::onNetworkError);
}

void FtpManager::createDirectory(const QString &dirPath)
{
    QUrl url = createUrl(dirPath);
    QNetworkRequest request(url);
    
    // För att skapa en katalog med FTP, gör en PUT med en tom fil
    // och lägg till parametern "mkdir"
    url.setQuery("mkdir");
    request.setUrl(url);
    
    QNetworkReply *reply = m_manager->put(request, QByteArray());
    
    connect(reply, &QNetworkReply::finished, [=]() {
        reply->deleteLater();
        
        if (reply->error() == QNetworkReply::NoError) {
            emit directoryCreated(dirPath);
        } else {
            emit error(tr("Kunde inte skapa katalog: %1").arg(reply->errorString()));
        }
    });
    
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &FtpManager::onNetworkError);
}

void FtpManager::deleteFile(const QString &filePath)
{
    QUrl url = createUrl(filePath);
    QNetworkRequest request(url);
    
    QNetworkReply *reply = m_manager->deleteResource(request);
    
    connect(reply, &QNetworkReply::finished, [=]() {
        reply->deleteLater();
        
        if (reply->error() == QNetworkReply::NoError) {
            emit fileDeleted(filePath);
        } else {
            emit error(tr("Kunde inte radera fil: %1").arg(reply->errorString()));
        }
    });
    
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &FtpManager::onNetworkError);
}

void FtpManager::deleteDirectory(const QString &dirPath)
{
    QUrl url = createUrl(dirPath);
    url.setQuery("rmdir");
    QNetworkRequest request(url);
    
    QNetworkReply *reply = m_manager->deleteResource(request);
    
    connect(reply, &QNetworkReply::finished, [=]() {
        reply->deleteLater();
        
        if (reply->error() == QNetworkReply::NoError) {
            emit directoryDeleted(dirPath);
        } else {
            emit error(tr("Kunde inte radera katalog: %1").arg(reply->errorString()));
        }
    });
    
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &FtpManager::onNetworkError);
}

void FtpManager::rename(const QString &oldPath, const QString &newPath)
{
    // För att byta namn med FTP, använd kommandot "rename"
    QUrl url = createUrl(oldPath);
    url.setQuery("rename=" + QUrl::toPercentEncoding(newPath));
    QNetworkRequest request(url);
    
    QNetworkReply *reply = m_manager->put(request, QByteArray());
    
    connect(reply, &QNetworkReply::finished, [=]() {
        reply->deleteLater();
        
        if (reply->error() == QNetworkReply::NoError) {
            emit renamed(oldPath, newPath);
        } else {
            emit error(tr("Kunde inte byta namn: %1").arg(reply->errorString()));
        }
    });
    
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &FtpManager::onNetworkError);
}

void FtpManager::onAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator)
{
    Q_UNUSED(reply);
    
    authenticator->setUser(m_username);
    authenticator->setPassword(m_password);
}

void FtpManager::onNetworkError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) {
        emit this->error(reply->errorString());
    } else {
        emit this->error(tr("Nätverksfel: %1").arg(error));
    }
}

void FtpManager::onUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    emit transferProgress(bytesSent, bytesTotal, m_currentUploadPath);
}

void FtpManager::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit transferProgress(bytesReceived, bytesTotal, m_currentDownloadPath);
}

void FtpManager::onListFinished()
{
    if (m_currentListReply->error() != QNetworkReply::NoError) {
        emit error(tr("Fel vid listning av katalog: %1").arg(m_currentListReply->errorString()));
        m_currentListReply->deleteLater();
        m_currentListReply = nullptr;
        return;
    }
    
    // Uppdatera anslutningsstatus
    if (!m_connected) {
        m_connected = true;
        emit connected();
    }
    
    // Uppdatera aktuell katalog
    QUrl url = m_currentListReply->url();
    m_currentDirectory = url.path();
    
    // Läs och tolka svaret
    QByteArray data = m_currentListReply->readAll();
    QList<ServerFileItem> items = parseDirectoryListing(data);
    
    // Skicka signal
    emit directoryListed(m_currentDirectory, items);
    
    // Städa upp
    m_currentListReply->deleteLater();
    m_currentListReply = nullptr;
}

void FtpManager::onUploadFinished()
{
    if (m_currentUploadReply->error() != QNetworkReply::NoError) {
        emit error(tr("Fel vid uppladdning av fil: %1").arg(m_currentUploadReply->errorString()));
    } else {
        emit uploadFinished(m_currentUploadPath);
    }
    
    // Städa upp
    m_currentUploadReply->deleteLater();
    m_currentUploadReply = nullptr;
    m_currentUploadPath.clear();
    m_currentLocalUploadPath.clear();
}

void FtpManager::onDownloadFinished()
{
    if (m_currentDownloadReply->error() != QNetworkReply::NoError) {
        emit error(tr("Fel vid nedladdning av fil: %1").arg(m_currentDownloadReply->errorString()));
    } else {
        // Spara data till fil
        QByteArray data = m_currentDownloadReply->readAll();
        QFile file(m_currentLocalDownloadPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(data);
            file.close();
            emit downloadFinished(m_currentDownloadPath);
        } else {
            emit error(tr("Kunde inte spara fil: %1").arg(file.errorString()));
        }
    }
    
    // Städa upp
    m_currentDownloadReply->deleteLater();
    m_currentDownloadReply = nullptr;
    m_currentDownloadPath.clear();
    m_currentLocalDownloadPath.clear();
}

QUrl FtpManager::createUrl(const QString &path) const
{
    QUrl url;
    url.setScheme("ftp");
    url.setHost(m_host);
    
    if (m_port != 21) {
        url.setPort(m_port);
    }
    
    QString urlPath = path;
    if (!urlPath.startsWith('/')) {
        urlPath = m_currentDirectory;
        if (!urlPath.endsWith('/')) {
            urlPath += '/';
        }
        urlPath += path;
    }
    
    url.setPath(urlPath);
    
    // Lägg inte till användarinformation, den hanteras av QAuthenticator
    
    return url;
}

QList<ServerFileItem> FtpManager::parseDirectoryListing(const QByteArray &data) const
{
    QList<ServerFileItem> items;
    
    // FTP-listning är vanligtvis i Unix-stil:
    // -rw-r--r-- 1 owner group    12345 Jan 01 12:34 filename.txt
    // drwxr-xr-x 2 owner group     4096 Jan 01 12:34 directory
    
    QString listing = QString::fromUtf8(data);
    QStringList lines = listing.split('\n', Qt::SkipEmptyParts);
    
    for (const QString &line : lines) {
        // Ignorera icke-listningsrader (t.ex. "total 123")
        if (line.startsWith("total ") || line.trimmed().isEmpty()) {
            continue;
        }
        
        // Tolka listningsraden
        QRegularExpression re("^([\\-dbclps])([rwxsStT\\-]{9})\\s+"    // Permissions
                             "(?:\\d+\\s+)?"                          // Link count (optional)
                             "(?:[^\\s]+\\s+)?"                       // Owner (optional)
                             "(?:[^\\s]+\\s+)?"                       // Group (optional)
                             "(\\d+)\\s+"                            // Size
                             "(?:(\\w{3})\\s+(\\d{1,2})\\s+"          // Month, Day
                             "(?:(\\d{4})|([0-9:]{4,5}))\\s+|"        // Year or Time
                             "(\\d{4})-(\\d{2})-(\\d{2})\\s+"         // ISO Date
                             "(\\d{2}):(\\d{2})\\s+)"                 // ISO Time
                             "(.+)$");                               // Filename
        
        QRegularExpressionMatch match = re.match(line);
        
        if (match.hasMatch()) {
            QString permissions = match.captured(1) + match.captured(2);
            bool isDirectory = (permissions.at(0) == 'd');
            qint64 size = match.captured(3).toLongLong();
            
            QDateTime lastModified;
            if (!match.captured(8).isEmpty()) {
                // ISO format
                int year = match.captured(8).toInt();
                int month = match.captured(9).toInt();
                int day = match.captured(10).toInt();
                int hour = match.captured(11).toInt();
                int minute = match.captured(12).toInt();
                lastModified = QDateTime(QDate(year, month, day), QTime(hour, minute));
            } else {
                // Unix format
                QDate date;
                QTime time;
                
                QStringList months = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                                    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
                int month = months.indexOf(match.captured(4)) + 1;
                int day = match.captured(5).toInt();
                
                if (!match.captured(6).isEmpty()) {
                    // År specificerat
                    int year = match.captured(6).toInt();
                    date = QDate(year, month, day);
                    time = QTime(0, 0);
                } else {
                    // Tid specificerad, år är nuvarande
                    QStringList timeParts = match.captured(7).split(':');
                    int hour = timeParts.at(0).toInt();
                    int minute = timeParts.at(1).toInt();
                    
                    int year = QDate::currentDate().year();
                    date = QDate(year, month, day);
                    time = QTime(hour, minute);
                    
                    // Om datumet är i framtiden, anta förra året
                    if (QDateTime(date, time) > QDateTime::currentDateTime().addDays(1)) {
                        date = QDate(year - 1, month, day);
                    }
                }
                
                lastModified = QDateTime(date, time);
            }
            
            QString name = match.captured(13);
            
            // Lägg till objektet i listan
            ServerFileItem item(name, isDirectory, size, permissions, lastModified);
            items.append(item);
        }
    }
    
    return items;
}

