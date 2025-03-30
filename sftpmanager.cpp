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
        emit downloadProgress(m_currentLocalPath, m_processedBytes, m_totalBytes);
        
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
        emit uploadProgress(m_currentRemotePath, m_processedBytes, m_totalBytes);
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
    
    // Lägg alltid till en indikator för överliggande katalog, förutom i roten
    if (path != "/") {
        entries.append("..");
    }
    
    // Standardkataloger som ska finnas i roten
    if (path == "/") {
        entries.append("public");
        entries.append("private");
        entries.append("uploads");
        entries.append("downloads");
        entries.append("docs");
        entries.append("welcome.txt");
        entries.append("readme.pdf");
        entries.append("example.zip");
    }
    // Innehåll i "public"-katalogen
    else if (path == "/public") {
        entries.append("shared_data.csv");
        entries.append("public_info.txt");
        entries.append("gallery");
        entries.append("documents");
    }
    // Innehåll i "private"-katalogen
    else if (path == "/private") {
        entries.append("personal");
        entries.append("work");
        entries.append("notes.txt");
        entries.append("todo.txt");
        entries.append("important.docx");
    }
    // Innehåll i "uploads"-katalogen
    else if (path == "/uploads") {
        entries.append("images");
        entries.append("videos");
        entries.append("documents");
        entries.append("temp");
    }
    // Innehåll i "downloads"-katalogen
    else if (path == "/downloads") {
        entries.append("software");
        entries.append("media");
        entries.append("archives");
        entries.append("sample_download.zip");
    }
    // Standardinnehåll för andra kataloger
    else {
        // Generera några slumpmässiga filer och mappar
        int numEntries = QRandomGenerator::global()->bounded(5, 15);
        
        for (int i = 0; i < numEntries; ++i) {
            if (QRandomGenerator::global()->bounded(2) == 0) {
                // Mapp
                QString folderName = QString("folder_%1").arg(QRandomGenerator::global()->bounded(1000));
                entries.append(folderName);
            } else {
                // Fil
                static const QStringList extensions = {".txt", ".pdf", ".docx", ".jpg", ".png", ".zip", ".mp3", ".mp4"};
                QString fileName = QString("file_%1%2")
                    .arg(QRandomGenerator::global()->bounded(1000))
                    .arg(extensions.at(QRandomGenerator::global()->bounded(extensions.size())));
                entries.append(fileName);
            }
        }
    }
    
    return entries;
} 