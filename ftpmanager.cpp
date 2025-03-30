// ftpmanager.cpp
#include "ftpmanager.h"

FtpManager::FtpManager(QObject *parent)
    : QObject(parent),
    m_networkManager(new QNetworkAccessManager(this)),
    m_port(21),
    m_connected(false),
    m_currentReply(nullptr),
    m_currentFile(nullptr)
{
    connect(m_networkManager, &QNetworkAccessManager::authenticationRequired,
            this, &FtpManager::onAuthenticationRequired);
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &FtpManager::onFinished);
}

FtpManager::~FtpManager()
{
    disconnectFromServer();
    delete m_networkManager;
}

bool FtpManager::connectToServer(const QString &host, int port, const QString &username, const QString &password)
{
    m_host = host;
    m_port = port;
    m_username = username;
    m_password = password;

    QUrl url;
    url.setScheme("ftp");
    url.setHost(host);
    url.setPort(port);
    url.setUserName(username);
    url.setPassword(password);
    url.setPath("/");

    QNetworkReply *reply = m_networkManager->get(QNetworkRequest(url));

    // Detta är en förenklad implementation - i en riktig klient skulle du behöva
    // använda en eventloop eller signals/slots för att hantera den asynkrona anslutningen

    // För demo-syfte antar vi att anslutningen lyckas om reply skapades
    if (reply) {
        m_connected = true;
        emit connected();
        return true;
    } else {
        emit error("Failed to connect to server");
        return false;
    }
}

void FtpManager::disconnectFromServer()
{
    if (m_connected) {
        m_connected = false;
        emit disconnected();
    }
}

bool FtpManager::isConnected() const
{
    return m_connected;
}

void FtpManager::listDirectory(const QString &path)
{
    if (!m_connected) {
        emit error("Not connected to server");
        return;
    }

    QUrl url;
    url.setScheme("ftp");
    url.setHost(m_host);
    url.setPort(m_port);
    url.setUserName(m_username);
    url.setPassword(m_password);
    url.setPath(path);

    QNetworkReply *reply = m_networkManager->get(QNetworkRequest(url));
    m_currentReply = reply;
}

void FtpManager::downloadFile(const QString &remotePath, const QString &localPath)
{
    if (!m_connected) {
        emit error(tr("Not connected to FTP server"));
        return;
    }

    QUrl url(QString("ftp://%1:%2%3").arg(m_host).arg(m_port).arg(remotePath));
    url.setUserName(m_username);
    url.setPassword(m_password);

    QNetworkRequest request(url);
    m_currentReply = m_networkManager->get(request);
    
    m_currentFile = new QFile(localPath);
    if (!m_currentFile->open(QIODevice::WriteOnly)) {
        emit error(tr("Could not open local file for writing"));
        return;
    }

    connect(m_currentReply, &QNetworkReply::readyRead, this, [this]() {
        if (m_currentFile) {
            m_currentFile->write(m_currentReply->readAll());
        }
    });

    connect(m_currentReply, &QNetworkReply::downloadProgress, this, 
            [this, localPath](qint64 bytesReceived, qint64 bytesTotal) {
        emit downloadProgress(localPath, bytesReceived, bytesTotal);
    });

    emit downloadStarted(QFileInfo(localPath).fileName());
}

void FtpManager::uploadFile(const QString &localPath, const QString &remotePath)
{
    if (!m_connected) {
        emit error(tr("Not connected to FTP server"));
        return;
    }

    QFile file(localPath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit error(tr("Could not open local file for reading"));
        return;
    }

    QUrl url(QString("ftp://%1:%2%3").arg(m_host).arg(m_port).arg(remotePath));
    url.setUserName(m_username);
    url.setPassword(m_password);

    QNetworkRequest request(url);
    m_currentReply = m_networkManager->put(request, &file);

    connect(m_currentReply, &QNetworkReply::uploadProgress, this, 
            [this, remotePath](qint64 bytesSent, qint64 bytesTotal) {
        emit uploadProgress(remotePath, bytesSent, bytesTotal);
    });

    emit uploadStarted(QFileInfo(localPath).fileName());
}

void FtpManager::onAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator)
{
    authenticator->setUser(m_username);
    authenticator->setPassword(m_password);
}

void FtpManager::onFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit error(reply->errorString());

        if (m_currentFile) {
            m_currentFile->close();
            delete m_currentFile;
            m_currentFile = nullptr;
        }

        if (reply == m_currentReply) {
            m_currentReply = nullptr;
        }

        reply->deleteLater();
        return;
    }

    // Hantera olika typer av operationer
    if (reply->operation() == QNetworkAccessManager::GetOperation) {
        if (m_currentFile) {
            // Detta var en filnedladdning
            m_currentFile->close();
            delete m_currentFile;
            m_currentFile = nullptr;
            emit downloadFinished(true);
        } else {
            // Detta var troligtvis en kataloglistning
            QByteArray data = reply->readAll();
            // Parsa katalogdata (detta är en enkel implementation)
            QString listing = QString::fromUtf8(data);
            QStringList entries = listing.split("\n", Qt::SkipEmptyParts);
            emit directoryListed(entries);
        }
    } else if (reply->operation() == QNetworkAccessManager::PutOperation) {
        // Detta var en filuppladdning
        if (m_currentFile) {
            m_currentFile->close();
            delete m_currentFile;
            m_currentFile = nullptr;
            emit uploadFinished(true);
        }
    }

    if (reply == m_currentReply) {
        m_currentReply = nullptr;
    }

    reply->deleteLater();
}
