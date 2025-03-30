#ifndef SFTPMANAGER_H
#define SFTPMANAGER_H

#include <QObject>
#include <QTimer>
#include <QRandomGenerator>
#include <QDateTime>
#include <QFile>

class SftpManager : public QObject
{
    Q_OBJECT
    
public:
    explicit SftpManager(QObject *parent = nullptr);
    ~SftpManager();
    
    bool connectToServer(const QString &host, int port, const QString &username, const QString &password);
    void disconnectFromServer();
    bool isConnected() const;
    void listDirectory(const QString &path);
    void downloadFile(const QString &remotePath, const QString &localPath);
    void uploadFile(const QString &localPath, const QString &remotePath);
    
signals:
    void connected();
    void disconnected();
    void error(const QString &errorMessage);
    void directoryListed(const QStringList &entries);
    void downloadStarted(const QString &filename);
    void downloadProgress(const QString &filePath, qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished(bool success);
    void uploadStarted(const QString &filename);
    void uploadProgress(const QString &filePath, qint64 bytesSent, qint64 bytesTotal);
    void uploadFinished(bool success);
    void logMessage(const QString &message);
    
private:
    bool m_connected;
    QString m_host;
    int m_port;
    QString m_username;
    QString m_password;
    
    QTimer *m_simulateTimer;
    QTimer *m_progressTimer;
    
    // Simulerade filöverföringsvariabler
    QString m_currentTransferFilename;
    QString m_currentLocalPath;
    QString m_currentRemotePath;
    qint64 m_totalBytes;
    qint64 m_processedBytes;
    bool m_isDownload;
    QFile *m_currentFile;
    
    // Simulerade operationer
    void simulateConnection();
    void simulateDirectoryListing(const QString &path);
    void simulateFileTransfer(bool isDownload, qint64 fileSize);
    void updateTransferProgress();
    void finishTransfer(bool success = true);
    
    // Hjälpfunktioner
    QStringList generateRandomDirListing(const QString &path);
};

#endif // SFTPMANAGER_H 