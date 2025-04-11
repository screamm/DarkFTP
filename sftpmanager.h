#ifndef SFTPMANAGER_H
#define SFTPMANAGER_H

#include <QObject>
#include <QSsh/sshconnection.h>
#include <QSsh/sftpchannel.h>
#include <QList>
#include "serverfileitem.h"

/**
 * @brief SftpManager hanterar anslutningar och filöverföringar med SFTP
 */
class SftpManager : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Standardkonstruktor
     * @param parent Förälderobjekt
     */
    explicit SftpManager(QObject *parent = nullptr);
    
    /**
     * @brief Destruktor
     */
    ~SftpManager();
    
    /**
     * @brief Anslut till SFTP-server
     * @param host Värdnamn eller IP-adress
     * @param username Användarnamn
     * @param password Lösenord
     * @param port Port (standard: 22)
     */
    void connectToHost(const QString &host, const QString &username, 
                       const QString &password, quint16 port = 22);
    
    /**
     * @brief Anslut till SFTP-server med SSH-nyckel
     * @param host Värdnamn eller IP-adress
     * @param username Användarnamn
     * @param privateKeyPath Sökväg till privat SSH-nyckel
     * @param passphrase Lösenfras för SSH-nyckeln (om den är krypterad)
     * @param password Lösenord (används endast om nyckelautentisering misslyckas)
     * @param port Port (standard: 22)
     * @return true om anslutningsförsöket kunde initieras, annars false
     */
    bool connectToHostWithKey(const QString &host, const QString &username,
                            const QString &privateKeyPath, const QString &passphrase,
                            const QString &password, quint16 port = 22);
    
    /**
     * @brief Koppla från SFTP-server
     */
    void disconnectFromHost();
    
    /**
     * @brief Lista innehållet i en katalog
     * @param path Sökväg att lista (tom sträng för hemkatalog)
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
     * @brief Kontrollera om ansluten
     * @return true om ansluten, annars false
     */
    bool isConnected() const;
    
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
    
    /**
     * @brief Signal som skickas när anslutningen har avbrutits
     */
    void disconnected();
    
    /**
     * @brief Signal som skickas när ett fel inträffar
     * @param errorString Felbeskrivning
     */
    void error(const QString &errorString);
    
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

private slots:
    /**
     * @brief Hantera när SSH-anslutningen har upprättats
     */
    void onSshConnectionEstablished();
    
    /**
     * @brief Hantera när SSH-anslutningen får ett fel
     * @param error Felet som uppstod
     */
    void onSshConnectionError(QSsh::SshError error);
    
    /**
     * @brief Hantera när SFTP-kanalen har öppnats
     */
    void onSftpChannelInitialized();
    
    /**
     * @brief Hantera när SFTP-kanalen får ett fel
     * @param errorMessage Felbeskrivning
     */
    void onSftpChannelError(const QString &errorMessage);
    
    /**
     * @brief Hantera när kataloglistning är klar
     * @param job Listningsjobbet
     * @param dirContent Kataloginnehåll
     * @param error Eventuellt fel
     */
    void onListDirJobFinished(QSsh::SftpJobId job, const QList<QSsh::SftpFileInfo> &dirContent, const QString &error);
    
    /**
     * @brief Hantera när filuppladdning är klar
     * @param job Uppladdningsjobbet
     * @param error Eventuellt fel
     */
    void onFileUploadJobFinished(QSsh::SftpJobId job, const QString &error);
    
    /**
     * @brief Hantera när filnedladdning är klar
     * @param job Nedladdningsjobbet
     * @param error Eventuellt fel
     */
    void onFileDownloadJobFinished(QSsh::SftpJobId job, const QString &error);
    
    /**
     * @brief Hantera när filöverföring rapporterar framsteg
     * @param job Filöverföringsjobbet
     * @param bytesSent Antal byte skickade
     * @param bytesTotal Totalt antal byte
     */
    void onTransferProgress(QSsh::SftpJobId job, quint64 bytesSent, quint64 bytesTotal);
    
    /**
     * @brief Hantera när katalog har skapats
     * @param job Jobbet för att skapa katalog
     * @param error Eventuellt fel
     */
    void onMkdirJobFinished(QSsh::SftpJobId job, const QString &error);
    
    /**
     * @brief Hantera när fil har raderats
     * @param job Jobbet för att radera fil
     * @param error Eventuellt fel
     */
    void onRemoveFileJobFinished(QSsh::SftpJobId job, const QString &error);
    
    /**
     * @brief Hantera när katalog har raderats
     * @param job Jobbet för att radera katalog
     * @param error Eventuellt fel
     */
    void onRemoveDirJobFinished(QSsh::SftpJobId job, const QString &error);
    
    /**
     * @brief Hantera när fil eller katalog har bytt namn
     * @param job Jobbet för att byta namn
     * @param error Eventuellt fel
     */
    void onRenameJobFinished(QSsh::SftpJobId job, const QString &error);

private:
    /**
     * @brief Konvertera QSsh::SftpFileInfo till ServerFileItem
     * @param fileInfo QSsh::SftpFileInfo att konvertera
     * @return Motsvarande ServerFileItem
     */
    ServerFileItem convertSftpFileInfo(const QSsh::SftpFileInfo &fileInfo) const;
    
    QSsh::SshConnectionParameters m_connectionParams;
    QSsh::SshConnection *m_sshConnection;
    QSsh::SftpChannel::Ptr m_sftpChannel;
    
    QString m_host;
    QString m_username;
    QString m_password;
    quint16 m_port;
    QString m_currentDirectory;
    bool m_connected;
    
    // Lagra information om pågående överföringar
    QSsh::SftpJobId m_currentListJob;
    QSsh::SftpJobId m_currentUploadJob;
    QSsh::SftpJobId m_currentDownloadJob;
    QSsh::SftpJobId m_currentMkdirJob;
    QSsh::SftpJobId m_currentRemoveFileJob;
    QSsh::SftpJobId m_currentRemoveDirJob;
    QSsh::SftpJobId m_currentRenameJob;
    
    QString m_currentUploadPath;
    QString m_currentDownloadPath;
    QString m_currentLocalUploadPath;
    QString m_currentLocalDownloadPath;
    QString m_currentMkdirPath;
    QString m_currentRemoveFilePath;
    QString m_currentRemoveDirPath;
    QString m_currentRenameSrcPath;
    QString m_currentRenameDstPath;
    QString m_currentListPath;
};

#endif // SFTPMANAGER_H 