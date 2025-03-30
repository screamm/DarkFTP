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
        emit error("Not connected to server");
        return;
    }

    QUrl url;
    url.setScheme("ftp");
    url.setHost(m_host);
    url.setPort(m_port);
    url.setUserName(m_username);
    url.setPassword(m_password);
    url.setPath(remotePath);

    QNetworkReply *reply = m_networkManager->get(QNetworkRequest(url));
    m_currentReply = reply;

    // Skapa fil för att spara data
    m_currentFile = new QFile(localPath);
    if (!m_currentFile->open(QIODevice::WriteOnly)) {
        emit error("Could not open local file for writing");
        reply->abort();
        delete m_currentFile;
        m_currentFile = nullptr;
        return;
    }

    connect(reply, &QNetworkReply::downloadProgress,
            this, &FtpManager::downloadProgress);

    connect(reply, &QNetworkReply::readyRead, [this, reply]() {
        if (m_currentFile) {
            m_currentFile->write(reply->readAll());
        }
    });

    emit downloadStarted(QFileInfo(localPath).fileName());
}

void FtpManager::uploadFile(const QString &localPath, const QString &remotePath)
{
    if (!m_connected) {
        emit error("Not connected to server");
        return;
    }

    // Öppna den lokala filen för läsning
    QFile *file = new QFile(localPath);
    if (!file->open(QIODevice::ReadOnly)) {
        emit error("Could not open local file for reading");
        delete file;
        return;
    }

    QUrl url;
    url.setScheme("ftp");
    url.setHost(m_host);
    url.setPort(m_port);
    url.setUserName(m_username);
    url.setPassword(m_password);
    url.setPath(remotePath);

    QNetworkRequest request(url);
    QNetworkReply *reply = m_networkManager->put(request, file);
    m_currentReply = reply;
    m_currentFile = file;

    connect(reply, &QNetworkReply::uploadProgress,
            this, &FtpManager::uploadProgress);

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
