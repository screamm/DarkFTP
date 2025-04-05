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
#include <QList>
#include "serverfileitem.h"

/**
 * @brief FtpManager hanterar anslutningar och filöverföringar med FTP
 */
class FtpManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Standardkonstruktor
     * @param parent Förälderobjekt
     */
    explicit FtpManager(QObject *parent = nullptr);
    ~FtpManager();

    /**
     * @brief Anslut till FTP-server
     * @param host Värdnamn eller IP-adress
     * @param username Användarnamn
     * @param password Lösenord
     * @param port Port (standard: 21)
     */
    void connectToHost(const QString &host, const QString &username, const QString &password, quint16 port = 21);
    void disconnectFromHost();

    bool isConnected() const;

    /**
     * @brief Lista innehållet i en katalog
     * @param path Sökväg att lista (tom sträng för rotkataloger)
     */
    void listDirectory(const QString &path = QString());

    /**
     * @brief Ladda upp en fil
     * @param localFilePath Lokal filsökväg
     * @param remoteFilePath Fjärrfilsökväg
     */
    void uploadFile(const QString &localFilePath, const QString &remoteFilePath);

    /**
     * @brief Ladda ner en fil
     * @param remoteFilePath Fjärrfilsökväg
     * @param localFilePath Lokal filsökväg
     */
    void downloadFile(const QString &remoteFilePath, const QString &localFilePath);

    /**
     * @brief Skapa en katalog
     * @param dirPath Sökväg till katalogen att skapa
     */
    void createDirectory(const QString &dirPath);

    /**
     * @brief Radera en fil
     * @param filePath Sökväg till filen att radera
     */
    void deleteFile(const QString &filePath);

    /**
     * @brief Radera en katalog
     * @param dirPath Sökväg till katalogen att radera
     */
    void deleteDirectory(const QString &dirPath);

    /**
     * @brief Byt namn på en fil eller katalog
     * @param oldPath Gammal sökväg
     * @param newPath Ny sökväg
     */
    void rename(const QString &oldPath, const QString &newPath);

    /**
     * @brief Hämta aktuell katalog
     * @return Aktuell katalog
     */
    QString currentDirectory() const;

signals:
    /**
     * @brief Signal som skickas när anslutningen har upprättats
     */
    void connected();
    void disconnected();
    void error(const QString &errorString);
    void commandSent(const QString &command);

    /**
     * @brief Signal som skickas när kataloglistan är klar
     * @param path Katalogen som listades
     * @param items Listan med filer och kataloger
     */
    void directoryListed(const QString &path, const QList<ServerFileItem> &items);

    /**
     * @brief Signal som skickas under filöverföring
     * @param bytesSent Antal byte skickade
     * @param bytesTotal Totalt antal byte
     * @param filePath Filsökväg som överförs
     */
    void transferProgress(qint64 bytesSent, qint64 bytesTotal, const QString &filePath);

    /**
     * @brief Signal som skickas när filuppladdning är klar
     * @param filePath Sökväg till den uppladdade filen
     */
    void uploadFinished(const QString &filePath);

    /**
     * @brief Signal som skickas när filnedladdning är klar
     * @param filePath Sökväg till den nedladdade filen
     */
    void downloadFinished(const QString &filePath);

    /**
     * @brief Signal som skickas när en katalog har skapats
     * @param dirPath Sökväg till den skapade katalogen
     */
    void directoryCreated(const QString &dirPath);

    /**
     * @brief Signal som skickas när en fil har raderats
     * @param filePath Sökväg till den raderade filen
     */
    void fileDeleted(const QString &filePath);

    /**
     * @brief Signal som skickas när en katalog har raderats
     * @param dirPath Sökväg till den raderade katalogen
     */
    void directoryDeleted(const QString &dirPath);

    /**
     * @brief Signal som skickas när en fil eller katalog har bytt namn
     * @param oldPath Gammal sökväg
     * @param newPath Ny sökväg
     */
    void renamed(const QString &oldPath, const QString &newPath);

private:
    QNetworkAccessManager *m_networkManager;
    QString m_host;
    quint16 m_port;
    QString m_username;
    QString m_password;
    bool m_connected;
    QString m_currentDirectory;

    QNetworkReply *m_currentReply;
    QFile *m_currentFile;

    QNetworkReply *m_currentListReply;
    QNetworkReply *m_currentUploadReply;
    QNetworkReply *m_currentDownloadReply;

    QString m_currentUploadPath;
    QString m_currentDownloadPath;
    QString m_currentLocalUploadPath;
    QString m_currentLocalDownloadPath;

private slots:
    void onAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);
    void onFinished(QNetworkReply *reply);
    void onNetworkError(QNetworkReply::NetworkError error);
    void onUploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onListFinished();
    void onUploadFinished();
    void onDownloadFinished();
};

#endif // FTPMANAGER_H
