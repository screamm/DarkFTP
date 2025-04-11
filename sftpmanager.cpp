#include "mainwindow.h"
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QDir>
#include <QDateTime>
#include <QRandomGenerator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSsh>

// SftpManager-implementation
// Detta är en simulerad implementation av SFTP-funktionalitet
// I en riktig implementation skulle vi använda ett bibliotek som libssh2

SftpManager::SftpManager(QObject *parent)
    : QObject(parent)
    , m_sshConnection(nullptr)
    , m_port(22)
    , m_connected(false)
    , m_currentDirectory("")
    , m_currentListJob(0)
    , m_currentUploadJob(0)
    , m_currentDownloadJob(0)
    , m_currentMkdirJob(0)
    , m_currentRemoveFileJob(0)
    , m_currentRemoveDirJob(0)
    , m_currentRenameJob(0)
{
}

SftpManager::~SftpManager()
{
    disconnectFromHost();
}

void SftpManager::connectToHost(const QString &host, const QString &username, 
                               const QString &password, quint16 port)
{
    if (m_connected || m_sshConnection) {
        disconnectFromHost();
    }
    
    // Spara anslutningsinformationen
    m_host = host;
    m_username = username;
    m_password = password;
    m_port = port;
    
    // Konfigurera SSH-anslutningsparametrar
    m_connectionParams.host = host;
    m_connectionParams.port = port;
    m_connectionParams.userName = username;
    m_connectionParams.password = password;
    m_connectionParams.authenticationType = QSsh::SshConnectionParameters::AuthenticationTypePassword;
    m_connectionParams.timeout = 30;
    m_connectionParams.options = QSsh::SshConnectionParameters::NoOption;
    
    // Skapa SSH-anslutning
    m_sshConnection = new QSsh::SshConnection(m_connectionParams, this);
    
    // Anslut signaler
    connect(m_sshConnection, &QSsh::SshConnection::connected, this, &SftpManager::onSshConnectionEstablished);
    connect(m_sshConnection, &QSsh::SshConnection::error, this, &SftpManager::onSshConnectionError);
    
    // Starta anslutningen
    m_sshConnection->connectToHost();
}

bool SftpManager::connectToHostWithKey(const QString &host, const QString &username,
                                     const QString &privateKeyPath, const QString &passphrase,
                                     const QString &password, quint16 port)
{
    if (m_connected || m_sshConnection) {
        disconnectFromHost();
    }
    
    // Kontrollera att nyckelfilen finns
    QFileInfo keyFileInfo(privateKeyPath);
    if (!keyFileInfo.exists() || !keyFileInfo.isReadable()) {
        emit error(tr("SSH-nyckelfil finns inte eller är inte läsbar: %1").arg(privateKeyPath));
        return false;
    }
    
    // Spara anslutningsinformationen
    m_host = host;
    m_username = username;
    m_password = password; // Endast för fallback-autentisering
    m_port = port;
    
    // Konfigurera SSH-anslutningsparametrar
    m_connectionParams.host = host;
    m_connectionParams.port = port;
    m_connectionParams.userName = username;
    
    // Ställ in nyckelbaserad autentisering
    if (!password.isEmpty()) {
        // Använd både lösenord och nyckel om båda anges
        m_connectionParams.authenticationType = QSsh::SshConnectionParameters::AuthenticationTypeAll;
        m_connectionParams.password = password;
    } else {
        // Använd endast nyckel
        m_connectionParams.authenticationType = QSsh::SshConnectionParameters::AuthenticationTypePublicKey;
    }
    
    // Sätt sökväg till privat nyckel och lösenfras
    m_connectionParams.privateKeyFile = privateKeyPath;
    
    if (!passphrase.isEmpty()) {
        m_connectionParams.passphrase = passphrase;
    }
    
    m_connectionParams.timeout = 30;
    m_connectionParams.options = QSsh::SshConnectionParameters::NoOption;
    
    // Logga till konsol för felsökning
    qDebug() << "Ansluter till SFTP med nyckel:"
             << "host=" << host
             << "port=" << port
             << "username=" << username
             << "keyPath=" << privateKeyPath
             << "hasPassphrase=" << !passphrase.isEmpty()
             << "hasPassword=" << !password.isEmpty();
    
    // Skapa SSH-anslutning
    m_sshConnection = new QSsh::SshConnection(m_connectionParams, this);
    
    // Anslut signaler
    connect(m_sshConnection, &QSsh::SshConnection::connected, this, &SftpManager::onSshConnectionEstablished);
    connect(m_sshConnection, &QSsh::SshConnection::error, this, &SftpManager::onSshConnectionError);
    
    // Starta anslutningen
    m_sshConnection->connectToHost();
    
    return true;
}

void SftpManager::disconnectFromHost()
{
    if (m_sftpChannel) {
        m_sftpChannel->closeChannel();
        m_sftpChannel.clear();
    }
    
    if (m_sshConnection) {
        m_sshConnection->disconnectFromHost();
        disconnect(m_sshConnection, nullptr, this, nullptr);
        m_sshConnection->deleteLater();
        m_sshConnection = nullptr;
    }
    
    m_connected = false;
    emit disconnected();
}

bool SftpManager::isConnected() const
{
    return m_connected;
}

QString SftpManager::currentDirectory() const
{
    return m_currentDirectory;
}

void SftpManager::listDirectory(const QString &path)
{
    if (!m_connected || !m_sftpChannel) {
        emit error(tr("Inte ansluten till SFTP-server"));
        return;
    }
    
    QString dirPath = path;
    if (dirPath.isEmpty()) {
        dirPath = m_currentDirectory.isEmpty() ? "/" : m_currentDirectory;
    }
    
    m_currentListPath = dirPath;
    m_currentListJob = m_sftpChannel->listDirectory(dirPath);
    
    connect(m_sftpChannel.data(), &QSsh::SftpChannel::fileInfoAvailable, 
            this, &SftpManager::onListDirJobFinished);
}

void SftpManager::uploadFile(const QString &localFilePath, const QString &remoteFilePath)
{
    if (!m_connected || !m_sftpChannel) {
        emit error(tr("Inte ansluten till SFTP-server"));
        return;
    }
    
    QFile *file = new QFile(localFilePath, this);
    if (!file->open(QIODevice::ReadOnly)) {
        emit error(tr("Kunde inte öppna lokal fil: %1").arg(file->errorString()));
        file->deleteLater();
        return;
    }
    
    m_currentLocalUploadPath = localFilePath;
    m_currentUploadPath = remoteFilePath;
    m_currentUploadJob = m_sftpChannel->uploadFile(file->handle(), remoteFilePath, file->size());
    
    connect(m_sftpChannel.data(), &QSsh::SftpChannel::finished, 
            this, &SftpManager::onFileUploadJobFinished);
            
    connect(m_sftpChannel.data(), &QSsh::SftpChannel::transferProgress, 
            this, &SftpManager::onTransferProgress);
    
    // Filobjektet kommer att raderas i onFileUploadJobFinished
    file->setParent(this);
}

void SftpManager::downloadFile(const QString &remoteFilePath, const QString &localFilePath)
{
    if (!m_connected || !m_sftpChannel) {
        emit error(tr("Inte ansluten till SFTP-server"));
        return;
    }
    
    // Skapa katalogstruktur om den inte finns
    QFileInfo fileInfo(localFilePath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QFile *file = new QFile(localFilePath, this);
    if (!file->open(QIODevice::WriteOnly)) {
        emit error(tr("Kunde inte öppna lokal fil för skrivning: %1").arg(file->errorString()));
        file->deleteLater();
        return;
    }
    
    m_currentDownloadPath = remoteFilePath;
    m_currentLocalDownloadPath = localFilePath;
    m_currentDownloadJob = m_sftpChannel->downloadFile(remoteFilePath, file->handle());
    
    connect(m_sftpChannel.data(), &QSsh::SftpChannel::finished, 
            this, &SftpManager::onFileDownloadJobFinished);
            
    connect(m_sftpChannel.data(), &QSsh::SftpChannel::transferProgress, 
            this, &SftpManager::onTransferProgress);
    
    // Filobjektet kommer att raderas i onFileDownloadJobFinished
    file->setParent(this);
}

void SftpManager::createDirectory(const QString &dirPath)
{
    if (!m_connected || !m_sftpChannel) {
        emit error(tr("Inte ansluten till SFTP-server"));
        return;
    }
    
    m_currentMkdirPath = dirPath;
    m_currentMkdirJob = m_sftpChannel->createDirectory(dirPath);
    
    connect(m_sftpChannel.data(), &QSsh::SftpChannel::finished, 
            this, &SftpManager::onMkdirJobFinished);
}

void SftpManager::deleteFile(const QString &filePath)
{
    if (!m_connected || !m_sftpChannel) {
        emit error(tr("Inte ansluten till SFTP-server"));
        return;
    }
    
    m_currentRemoveFilePath = filePath;
    m_currentRemoveFileJob = m_sftpChannel->removeFile(filePath);
    
    connect(m_sftpChannel.data(), &QSsh::SftpChannel::finished, 
            this, &SftpManager::onRemoveFileJobFinished);
}

void SftpManager::deleteDirectory(const QString &dirPath)
{
    if (!m_connected || !m_sftpChannel) {
        emit error(tr("Inte ansluten till SFTP-server"));
        return;
    }
    
    m_currentRemoveDirPath = dirPath;
    m_currentRemoveDirJob = m_sftpChannel->removeDirectory(dirPath);
    
    connect(m_sftpChannel.data(), &QSsh::SftpChannel::finished, 
            this, &SftpManager::onRemoveDirJobFinished);
}

void SftpManager::rename(const QString &oldPath, const QString &newPath)
{
    if (!m_connected || !m_sftpChannel) {
        emit error(tr("Inte ansluten till SFTP-server"));
        return;
    }
    
    m_currentRenameSrcPath = oldPath;
    m_currentRenameDstPath = newPath;
    m_currentRenameJob = m_sftpChannel->renameFile(oldPath, newPath);
    
    connect(m_sftpChannel.data(), &QSsh::SftpChannel::finished, 
            this, &SftpManager::onRenameJobFinished);
}

void SftpManager::onSshConnectionEstablished()
{
    // SSH-anslutningen är upprättad, nu kan vi öppna en SFTP-kanal
    m_sftpChannel = m_sshConnection->createSftpChannel();
    
    if (!m_sftpChannel) {
        emit error(tr("Kunde inte skapa SFTP-kanal"));
        disconnectFromHost();
        return;
    }
    
    connect(m_sftpChannel.data(), &QSsh::SftpChannel::initialized, 
            this, &SftpManager::onSftpChannelInitialized);
            
    connect(m_sftpChannel.data(), &QSsh::SftpChannel::channelError, 
            this, &SftpManager::onSftpChannelError);
    
    m_sftpChannel->initialize();
}

void SftpManager::onSshConnectionError(QSsh::SshError error)
{
    QString errorString;
    
    switch (error) {
        case QSsh::SshNoError:
            return;
        case QSsh::SshSocketError:
            errorString = tr("Socketfel vid anslutning");
            break;
        case QSsh::SshTimeoutError:
            errorString = tr("Tidsgräns överskriden");
            break;
        case QSsh::SshAuthenticationError:
            errorString = tr("Autentiseringsfel");
            break;
        case QSsh::SshClosedByServerError:
            errorString = tr("Anslutningen stängdes av servern");
            break;
        case QSsh::SshProtocolError:
            errorString = tr("Protokollfel");
            break;
        default:
            errorString = tr("Okänt SSH-fel");
            break;
    }
    
    emit this->error(errorString);
    disconnectFromHost();
}

void SftpManager::onSftpChannelInitialized()
{
    m_connected = true;
    emit connected();
    
    // Lista roten som första åtgärd för att hämta hemkatalogen
    listDirectory("/");
}

void SftpManager::onSftpChannelError(const QString &errorMessage)
{
    emit error(tr("SFTP-fel: %1").arg(errorMessage));
}

void SftpManager::onListDirJobFinished(QSsh::SftpJobId job, const QList<QSsh::SftpFileInfo> &dirContent, const QString &error)
{
    if (job != m_currentListJob) {
        return;
    }
    
    disconnect(m_sftpChannel.data(), &QSsh::SftpChannel::fileInfoAvailable, 
              this, &SftpManager::onListDirJobFinished);
    
    if (!error.isEmpty()) {
        emit this->error(tr("Kunde inte lista katalog: %1").arg(error));
        return;
    }
    
    // Uppdatera aktuell katalog
    m_currentDirectory = m_currentListPath;
    
    // Konvertera SFTP-filinformation till ServerFileItem
    QList<ServerFileItem> items;
    for (const QSsh::SftpFileInfo &fileInfo : dirContent) {
        // Ignorera "." och ".." för att hålla konsekvent med FTP
        if (fileInfo.name == "." || fileInfo.name == "..") {
            continue;
        }
        
        items.append(convertSftpFileInfo(fileInfo));
    }
    
    emit directoryListed(m_currentListPath, items);
}

void SftpManager::onFileUploadJobFinished(QSsh::SftpJobId job, const QString &error)
{
    if (job != m_currentUploadJob) {
        return;
    }
    
    disconnect(m_sftpChannel.data(), &QSsh::SftpChannel::finished, 
              this, &SftpManager::onFileUploadJobFinished);
    
    disconnect(m_sftpChannel.data(), &QSsh::SftpChannel::transferProgress, 
              this, &SftpManager::onTransferProgress);
    
    if (!error.isEmpty()) {
        emit this->error(tr("Kunde inte ladda upp fil: %1").arg(error));
    } else {
        emit uploadFinished(m_currentUploadPath);
    }
    
    m_currentUploadJob = 0;
    m_currentUploadPath.clear();
    m_currentLocalUploadPath.clear();
}

void SftpManager::onFileDownloadJobFinished(QSsh::SftpJobId job, const QString &error)
{
    if (job != m_currentDownloadJob) {
        return;
    }
    
    disconnect(m_sftpChannel.data(), &QSsh::SftpChannel::finished, 
              this, &SftpManager::onFileDownloadJobFinished);
    
    disconnect(m_sftpChannel.data(), &QSsh::SftpChannel::transferProgress, 
              this, &SftpManager::onTransferProgress);
    
    if (!error.isEmpty()) {
        emit this->error(tr("Kunde inte ladda ner fil: %1").arg(error));
    } else {
        emit downloadFinished(m_currentDownloadPath);
    }
    
    m_currentDownloadJob = 0;
    m_currentDownloadPath.clear();
    m_currentLocalDownloadPath.clear();
}

void SftpManager::onTransferProgress(QSsh::SftpJobId job, quint64 bytesSent, quint64 bytesTotal)
{
    QString filePath;
    
    if (job == m_currentUploadJob) {
        filePath = m_currentUploadPath;
    } else if (job == m_currentDownloadJob) {
        filePath = m_currentDownloadPath;
    } else {
        return;
    }
    
    emit transferProgress(bytesSent, bytesTotal, filePath);
}

void SftpManager::onMkdirJobFinished(QSsh::SftpJobId job, const QString &error)
{
    if (job != m_currentMkdirJob) {
        return;
    }
    
    disconnect(m_sftpChannel.data(), &QSsh::SftpChannel::finished, 
              this, &SftpManager::onMkdirJobFinished);
    
    if (!error.isEmpty()) {
        emit this->error(tr("Kunde inte skapa katalog: %1").arg(error));
    } else {
        emit directoryCreated(m_currentMkdirPath);
    }
    
    m_currentMkdirJob = 0;
    m_currentMkdirPath.clear();
}

void SftpManager::onRemoveFileJobFinished(QSsh::SftpJobId job, const QString &error)
{
    if (job != m_currentRemoveFileJob) {
        return;
    }
    
    disconnect(m_sftpChannel.data(), &QSsh::SftpChannel::finished, 
              this, &SftpManager::onRemoveFileJobFinished);
    
    if (!error.isEmpty()) {
        emit this->error(tr("Kunde inte radera fil: %1").arg(error));
    } else {
        emit fileDeleted(m_currentRemoveFilePath);
    }
    
    m_currentRemoveFileJob = 0;
    m_currentRemoveFilePath.clear();
}

void SftpManager::onRemoveDirJobFinished(QSsh::SftpJobId job, const QString &error)
{
    if (job != m_currentRemoveDirJob) {
        return;
    }
    
    disconnect(m_sftpChannel.data(), &QSsh::SftpChannel::finished, 
              this, &SftpManager::onRemoveDirJobFinished);
    
    if (!error.isEmpty()) {
        emit this->error(tr("Kunde inte radera katalog: %1").arg(error));
    } else {
        emit directoryDeleted(m_currentRemoveDirPath);
    }
    
    m_currentRemoveDirJob = 0;
    m_currentRemoveDirPath.clear();
}

void SftpManager::onRenameJobFinished(QSsh::SftpJobId job, const QString &error)
{
    if (job != m_currentRenameJob) {
        return;
    }
    
    disconnect(m_sftpChannel.data(), &QSsh::SftpChannel::finished, 
              this, &SftpManager::onRenameJobFinished);
    
    if (!error.isEmpty()) {
        emit this->error(tr("Kunde inte byta namn: %1").arg(error));
    } else {
        emit renamed(m_currentRenameSrcPath, m_currentRenameDstPath);
    }
    
    m_currentRenameJob = 0;
    m_currentRenameSrcPath.clear();
    m_currentRenameDstPath.clear();
}

ServerFileItem SftpManager::convertSftpFileInfo(const QSsh::SftpFileInfo &fileInfo) const
{
    bool isDirectory = (fileInfo.type == QSsh::SftpFileInfo::Directory);
    QString permissions = "";
    
    // Skapa permissions-sträng i Unix-stil för att behålla samma gränssnitt som FTP
    permissions += isDirectory ? "d" : "-";
    
    // Ägare
    permissions += (fileInfo.permissions & QSsh::SftpFileInfo::OwnerRead) ? "r" : "-";
    permissions += (fileInfo.permissions & QSsh::SftpFileInfo::OwnerWrite) ? "w" : "-";
    permissions += (fileInfo.permissions & QSsh::SftpFileInfo::OwnerExec) ? "x" : "-";
    
    // Grupp
    permissions += (fileInfo.permissions & QSsh::SftpFileInfo::GroupRead) ? "r" : "-";
    permissions += (fileInfo.permissions & QSsh::SftpFileInfo::GroupWrite) ? "w" : "-";
    permissions += (fileInfo.permissions & QSsh::SftpFileInfo::GroupExec) ? "x" : "-";
    
    // Alla
    permissions += (fileInfo.permissions & QSsh::SftpFileInfo::OthersRead) ? "r" : "-";
    permissions += (fileInfo.permissions & QSsh::SftpFileInfo::OthersWrite) ? "w" : "-";
    permissions += (fileInfo.permissions & QSsh::SftpFileInfo::OthersExec) ? "x" : "-";
    
    return ServerFileItem(fileInfo.name, isDirectory, fileInfo.size, permissions, fileInfo.mtime);
} 