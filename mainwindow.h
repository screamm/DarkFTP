#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QTreeView>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QFileSystemModel>
#include <QStandardItemModel>
#include <QComboBox>
#include <QSettings>
#include <QListWidget>
#include <QTextEdit>
#include <QTabWidget>
#include "ftpmanager.h"
#include "sftpmanager.h"

// Forward-deklaration
class SftpManager;

// Enum för olika anslutningstyper
enum class ConnectionType {
    FTP,
    SFTP
};

// Struktur för att hålla anslutningsinformation
struct ConnectionInfo {
    QString name;
    ConnectionType type;
    QString host;
    int port;
    QString username;
    QString password;
    
    // Operatörer för jämförelse
    bool operator==(const ConnectionInfo& other) const {
        return name == other.name &&
               type == other.type &&
               host == other.host &&
               port == other.port &&
               username == other.username;
    }
};

// Enum för olika tema
enum class AppTheme {
    DarkTermius,
    RetroBlue,
    Steampunk,
    Hacker,
    Nordisk
};

// Enum för dragfunktionalitet
enum class DragDropAction {
    None,
    Upload,
    Download
};

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // Händelsehanterare för drag och drop
    bool eventFilter(QObject* watched, QEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

public slots:
    // Temahantering
    void setTheme(AppTheme theme);
    void switchTheme(int themeIndex);
    
    // Loggfunktioner
    void appendToLog(const QString &message);
    void clearLog();

private slots:
    void connectToFtp();
    void disconnectFromFtp();
    void browseLocalDirectory();
    void updateRemoteDirectory();
    void uploadFile();
    void downloadFile();
    void onFtpConnected();
    void onFtpDisconnected();
    void onFtpError(const QString &errorMessage);
    void onDirectoryListed(const QStringList &entries);
    void onDownloadFinished(bool success);
    void onUploadFinished(bool success);
    void onLocalDirectorySelected(const QModelIndex &index);
    void onRemoteDirectorySelected(const QModelIndex &index);
    
    // Anslutningshantering
    void showConnectionDialog();
    void setupConnectionDialog();
    void loadConnection(int index);
    void saveConnection();
    void deleteConnection();
    void loadSettings();
    void saveSettings();
    void updateConnectionsList();
    void showSavedConnectionsDialog();
    
    // Drag och Drop
    void handleLocalDrag(const QModelIndex &index);
    void handleRemoteDrag(const QModelIndex &index);
    void handleDrop(const QModelIndex &index, DragDropAction action);

    // Slots för överföringsframsteg
    void onFtpUploadProgress(const QString &filePath, qint64 bytesSent, qint64 bytesTotal);
    void onFtpDownloadProgress(const QString &filePath, qint64 bytesReceived, qint64 bytesTotal);

private:
    Ui::MainWindow *ui;
    
    // Hanterare för anslutningar
    FtpManager *m_ftpManager;
    SftpManager *m_sftpManager;
    ConnectionType m_activeConnectionType;
    
    // Modeller för lokal och fjärrfilvisning
    QFileSystemModel *m_localFileModel;
    QStandardItemModel *m_remoteFileModel;
    
    // Drag och drop-hantering
    DragDropAction m_currentDragAction;
    QString m_dragSourcePath;
    
    // Vy-komponenter
    QTreeView *m_localView;
    QTreeView *m_remoteView;
    QLineEdit *m_localPathEdit;
    QLineEdit *m_remotePathEdit;
    QPushButton *m_uploadButton;
    QPushButton *m_downloadButton;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QTextEdit *m_logTextEdit;
    QComboBox *m_themeComboBox;
    
    // Anslutningsdialogens komponenter
    QDialog *m_connectionDialog;
    QLineEdit *m_serverInput;
    QLineEdit *m_usernameInput;
    QLineEdit *m_passwordInput;
    QLineEdit *m_portInput;
    QComboBox *m_connectionTypeCombo;
    QListWidget *m_savedConnectionsList = nullptr;
    QPushButton *m_connectButton;
    QPushButton *m_disconnectButton;
    
    // Sökvägar
    QString m_currentLocalPath;
    QString m_currentRemotePath;
    
    // Lista över sparade anslutningar
    QList<ConnectionInfo> m_savedConnections;
    ConnectionInfo m_currentConnection;
    
    // Setup-funktioner
    void setupUi();
    void setupModels();
    void setupConnections();
    void createMenus();
    
    // Hjälpfunktioner för UI
    void updateUIState();
    void populateRemoteFileModel(const QStringList &entries);
    QString formatFileSize(qint64 bytes) const;
};

#endif // MAINWINDOW_H
