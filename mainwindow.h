#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <QFileSystemModel>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QSplitter>
#include <QTextEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QStatusBar>
#include <QSettings>
#include <QCloseEvent>
#include <QMenu>
#include <QMenuBar>
#include <QTabWidget>
#include <QTabBar>
#include <QMimeDatabase>
#include <QMap>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QColorDialog>
#include <QCheckBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QDialogButtonBox>

#include "ftpmanager.h"
#include "sftpmanager.h"
#include "connection.h"
#include "connectiondialog.h"
#include "serverfileitem.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(QString statusMessage READ statusMessage WRITE setStatusMessage NOTIFY statusMessageChanged)

public:
    // Datastruktur för att spara information om varje flik
    struct TabInfo {
        Connection connectionInfo;
        QString currentRemotePath;
        QWidget* contentWidget;
        QSplitter* splitter;
        QTreeView* localView;
        QLineEdit* localPathEdit;
        QTreeView* remoteView;
        QLineEdit* remotePathEdit;
        QFileSystemModel* localFileModel;
        QStandardItemModel* remoteFileModel;
        QPushButton* uploadButton;
        QPushButton* downloadButton;
        
        // Standardkonstruktor
        TabInfo() : contentWidget(nullptr), splitter(nullptr), localView(nullptr), 
                    localPathEdit(nullptr), remoteView(nullptr), remotePathEdit(nullptr),
                    localFileModel(nullptr), remoteFileModel(nullptr),
                    uploadButton(nullptr), downloadButton(nullptr) {}
    };
    
    // Tematyper
    enum ThemeType {
        ThemeLight,
        ThemeDark,
        ThemeCustom
    };
    
    // Handlingar för drag och drop
    enum DragDropAction {
        DragNone,
        DragUpload,
        DragDownload
    };

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString statusMessage() const;
    void setStatusMessage(const QString &message);

    Q_INVOKABLE void connectFromQml(const QString &protocolStr, const QString &host, int port, const QString &username, const QString &password);
    Q_INVOKABLE void connectFromQmlEx(const QString &protocolStr, const QString &host, int port, 
                                      const QString &username, const QString &password,
                                      int authMethodInt, const QString &keyPath, const QString &keyPassphrase);

private slots:
    void showConnectionDialog();
    void connectToServer(const Connection &connection);
    void disconnectFromServer();
    void showAboutDialog();
    void browseLocalDirectory();
    void updateLocalDirectory(const QString &path);
    void updateRemoteDirectory(const QString &path = QString());
    void uploadFile();
    void downloadFile();
    void onDirectoryListed(const QStringList &entries);
    void onFtpCommandSent(const QString &command);
    void clearLog();
    void showPreferences();
    void saveSettings();
    void setTheme(ThemeType theme);
    void addNewTab(const Connection &connection = Connection());
    void closeTab(int index);
    void switchTab(int index);
    void onRemoteSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onLocalSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void initializeFileIcons();
    void updateTabTitle(int index, const QString &title);
    void onTransferProgress(qint64 bytesSent, qint64 bytesTotal, const QString &file);
    void onConnected();
    void onDisconnected();
    void onError(const QString &errorMessage);

protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUi();
    void createMenus();
    void setupTab(TabInfo &tab);
    void loadSettings();
    QString getFileIconName(const QString &fileName, bool isDir);
    void processFtpEntry(const QString &entry);
    void processSftpEntry(const QString &entry);
    
    // Flikhanteringsvariabler
    QTabWidget* m_tabWidget;
    int m_currentTabIndex;
    QList<TabInfo> m_tabs;
    
    // Loggning och statusvariabler
    QTextEdit *m_logTextEdit;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    
    // Anslutningshanterare
    FtpManager *m_ftpManager;
    SftpManager *m_sftpManager;
    bool m_connected;
    Connection m_currentConnection;
    
    // Inställningar
    QSettings m_settings;
    ThemeType m_currentTheme;
    
    // Nedladdnings-/uppladdningsindikatorer
    DragDropAction m_currentDragAction;
    
    // Filtypsikoner
    QMimeDatabase m_mimeDb;
    QMap<QString, QIcon> m_fileTypeIcons;
    QString m_statusMessage;

signals:
    void statusMessageChanged();
};

#endif // MAINWINDOW_H
