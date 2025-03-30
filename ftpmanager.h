// ftpmanager.h
#ifndef FTPMANAGER_H
#define FTPMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QAuthenticator>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QFileSystemModel>

class FtpManager : public QObject
{
    Q_OBJECT

public:
    explicit FtpManager(QObject *parent = nullptr);
    ~FtpManager();

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

private:
    QNetworkAccessManager *m_networkManager;
    QString m_host;
    int m_port;
    QString m_username;
    QString m_password;
    bool m_connected;

    QNetworkReply *m_currentReply;
    QFile *m_currentFile;

private slots:
    void onAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);
    void onFinished(QNetworkReply *reply);
};

#endif // FTPMANAGER_H
