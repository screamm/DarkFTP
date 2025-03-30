#include "mainwindow.h"
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QDir>
#include <QDateTime>
#include <QRandomGenerator>

// SftpManager-implementation
// Detta är en simulerad implementation av SFTP-funktionalitet
// I en riktig implementation skulle vi använda ett bibliotek som libssh2

SftpManager::SftpManager(QObject *parent)
    : QObject(parent)
    , m_connected(false)
    , m_port(22)
    , m_simulateTimer(new QTimer(this))
    , m_progressTimer(new QTimer(this))
    , m_totalBytes(0)
    , m_processedBytes(0)
    , m_isDownload(false)
    , m_currentFile(nullptr)
{
    // Konfigurera timer för simulerade åtgärder
    m_simulateTimer->setSingleShot(true);
    m_progressTimer->setInterval(200); // 200ms uppdateringsintervall för förloppsindikatorn
    
    connect(m_progressTimer, &QTimer::timeout, this, &SftpManager::updateTransferProgress);
}

SftpManager::~SftpManager()
{
    if (m_connected) {
        disconnectFromServer();
    }
}

bool SftpManager::connectToServer(const QString &host, int port, const QString &username, const QString &password)
{
    if (m_connected) {
        disconnectFromServer();
    }
    
    emit logMessage(tr("Initierar SFTP-anslutning till %1:%2...").arg(host).arg(port));
    
    m_host = host;
    m_port = port;
    m_username = username;
    m_password = password;
    
    // Simulera anslutningsförlopp
    QTimer::singleShot(1500, this, &SftpManager::simulateConnection);
    
    return true; // Returnera true eftersom anslutningen har påbörjats
}

void SftpManager::disconnectFromServer()
{
    if (m_connected) {
        emit logMessage(tr("Kopplar från SFTP-anslutning..."));
        
        // Stoppa alla pågående simuleringar
        m_simulateTimer->stop();
        m_progressTimer->stop();
        
        // Stäng fil om öppen
        if (m_currentFile) {
            m_currentFile->close();
            delete m_currentFile;
            m_currentFile = nullptr;
        }
        
        m_connected = false;
        emit disconnected();
        emit logMessage(tr("Frånkopplad från SFTP-server"));
    }
}

bool SftpManager::isConnected() const
{
    return m_connected;
}

void SftpManager::listDirectory(const QString &path)
{
    if (!m_connected) {
        emit error(tr("Inte ansluten till SFTP-server"));
        return;
    }
    
    emit logMessage(tr("Listar katalog: %1").arg(path));
    
    // Direkt simulera kataloglistan (utan fördröjning)
    QStringList entries = generateRandomDirListing(path);
    emit directoryListed(entries);
    emit logMessage(tr("Kataloglistning klar: %1 (%2 objekt)").arg(path).arg(entries.size()));
}

void SftpManager::downloadFile(const QString &remotePath, const QString &localPath)
{
    if (!m_connected) {
        emit error(tr("Inte ansluten till SFTP-server"));
        return;
    }
    
    QFileInfo fileInfo(remotePath);
    m_currentTransferFilename = fileInfo.fileName();
    m_currentLocalPath = localPath;
    m_currentRemotePath = remotePath;
    m_isDownload = true;
    
    m_currentFile = new QFile(localPath);
    if (!m_currentFile->open(QIODevice::WriteOnly)) {
        emit error(tr("Kunde inte öppna lokal fil för skrivning: %1").arg(localPath));
        delete m_currentFile;
        m_currentFile = nullptr;
        return;
    }
    
    emit logMessage(tr("Laddar ner fil: %1 till %2").arg(remotePath).arg(localPath));
    emit downloadStarted(m_currentTransferFilename);
    
    // Simulera filstorlek: slumpmässig mellan 1MB och 20MB
    qint64 fileSize = QRandomGenerator::global()->bounded(1024*1024, 20*1024*1024);
    
    // Starta filöverföringssimulering
    simulateFileTransfer(true, fileSize);
}

void SftpManager::uploadFile(const QString &localPath, const QString &remotePath)
{
    if (!m_connected) {
        emit error(tr("Inte ansluten till SFTP-server"));
        return;
    }
    
    QFileInfo fileInfo(localPath);
    if (!fileInfo.exists()) {
        emit error(tr("Lokal fil finns inte: %1").arg(localPath));
        return;
    }
    
    m_currentTransferFilename = fileInfo.fileName();
    m_currentLocalPath = localPath;
    m_currentRemotePath = remotePath;
    m_isDownload = false;
    
    m_currentFile = new QFile(localPath);
    if (!m_currentFile->open(QIODevice::ReadOnly)) {
        emit error(tr("Kunde inte öppna lokal fil för läsning: %1").arg(localPath));
        delete m_currentFile;
        m_currentFile = nullptr;
        return;
    }
    
    emit logMessage(tr("Laddar upp fil: %1 till %2").arg(localPath).arg(remotePath));
    emit uploadStarted(m_currentTransferFilename);
    
    // Använd faktisk filstorlek
    qint64 fileSize = fileInfo.size();
    
    // Starta filöverföringssimulering
    simulateFileTransfer(false, fileSize);
}

void SftpManager::simulateConnection()
{
    // 10% sannolikhet för fel vid anslutning
    if (QRandomGenerator::global()->bounded(100) < 10) {
        emit error(tr("Kunde inte ansluta till SFTP-server: Anslutningen nekades"));
        return;
    }
    
    m_connected = true;
    emit connected();
    emit logMessage(tr("Ansluten till SFTP-server: %1").arg(m_host));
}

void SftpManager::simulateDirectoryListing(const QString &path)
{
    // 5% chans för fel vid kataloglistning
    if (QRandomGenerator::global()->bounded(100) < 5) {
        emit error(tr("Kunde inte lista katalog: %1 - Åtkomst nekad").arg(path));
        return;
    }
    
    QStringList entries = generateRandomDirListing(path);
    emit directoryListed(entries);
    emit logMessage(tr("Kataloglistning klar: %1 (%2 objekt)").arg(path).arg(entries.size()));
}

void SftpManager::simulateFileTransfer(bool isDownload, qint64 fileSize)
{
    m_totalBytes = fileSize;
    m_processedBytes = 0;
    m_isDownload = isDownload;
    
    // Starta förloppsindikatorn
    m_progressTimer->start();
}

void SftpManager::updateTransferProgress()
{
    // Simulera dataöverföring, 200KB-600KB per uppdatering
    qint64 chunkSize = QRandomGenerator::global()->bounded(200*1024, 600*1024);
    m_processedBytes += chunkSize;
    
    if (m_processedBytes >= m_totalBytes) {
        m_processedBytes = m_totalBytes;
        m_progressTimer->stop();
        
        // 5% chans för fel under överföring
        if (QRandomGenerator::global()->bounded(100) < 5) {
            finishTransfer(false);
            return;
        }
        
        finishTransfer(true);
        return;
    }
    
    // Emittera framstegssignal beroende på överföringstyp
    if (m_isDownload) {
        emit downloadProgress(m_processedBytes, m_totalBytes);
        
        // För nedladdningar, skriv slumpmässiga data till filen
        if (m_currentFile) {
            QByteArray randomData;
            randomData.resize(chunkSize);
            for (int i = 0; i < chunkSize; ++i) {
                randomData[i] = QRandomGenerator::global()->bounded(256);
            }
            m_currentFile->write(randomData);
        }
    } else {
        emit uploadProgress(m_processedBytes, m_totalBytes);
        // För uppladdningar behöver vi inte göra något med filen
    }
}

void SftpManager::finishTransfer(bool success)
{
    m_progressTimer->stop();
    
    if (m_currentFile) {
        m_currentFile->close();
        delete m_currentFile;
        m_currentFile = nullptr;
    }
    
    if (success) {
        if (m_isDownload) {
            emit downloadFinished(true);
            emit logMessage(tr("Nedladdning klar: %1").arg(m_currentTransferFilename));
        } else {
            emit uploadFinished(true);
            emit logMessage(tr("Uppladdning klar: %1").arg(m_currentTransferFilename));
        }
    } else {
        if (m_isDownload) {
            emit downloadFinished(false);
            emit error(tr("Nedladdning misslyckades: %1 - Anslutningen bröts").arg(m_currentTransferFilename));
        } else {
            emit uploadFinished(false);
            emit error(tr("Uppladdning misslyckades: %1 - Anslutningen bröts").arg(m_currentTransferFilename));
        }
        
        // Ta bort ofullständiga nedladdningsfiler
        if (m_isDownload && !m_currentLocalPath.isEmpty()) {
            QFile::remove(m_currentLocalPath);
        }
    }
}

QStringList SftpManager::generateRandomDirListing(const QString &path)
{
    QStringList entries;
    
    // Skapa några standardmappar för alla kataloger
    entries.append("d---------   2 user     group            0 Jan 01 2023 uploads");
    entries.append("d---------   2 user     group            0 Jan 01 2023 downloads");
    entries.append("d---------   2 user     group            0 Jan 01 2023 public");
    entries.append("d---------   2 user     group            0 Jan 01 2023 private");
    
    // Skapa några filer
    entries.append("----------   1 user     group     10240 Jan 01 2023 welcome.txt");
    entries.append("----------   1 user     group    102400 Jan 01 2023 readme.pdf");
    entries.append("----------   1 user     group   1024000 Jan 01 2023 example.zip");
    
    // Lägg till några specifika mappar för root
    if (path == "/") {
        entries.append("d---------   2 user     group            0 Jan 01 2023 home");
        entries.append("d---------   2 user     group            0 Jan 01 2023 var");
        entries.append("d---------   2 user     group            0 Jan 01 2023 etc");
    } 
    // Lägg till "upp en nivå" om vi inte är i roten
    else if (path != "/") {
        entries.prepend("d---------   2 user     group            0 Jan 01 2023 ..");
    }
    
    return entries;
} 