#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QStandardItem>
#include <QApplication>
#include <QStyle>
#include <QRegularExpression>
#include <QInputDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QDateTime>
#include <QActionGroup>
#include <QScreen>
#include <QCheckBox>
#include <QDebug>
#include <QDrag>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QTimer>
#include <QRandomGenerator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_ftpManager(new FtpManager(this))
    , m_sftpManager(new SftpManager(this))
    , m_activeConnectionType(ConnectionType::FTP)
    , m_localFileModel(new QFileSystemModel(this))
    , m_remoteFileModel(new QStandardItemModel(this))
    , m_currentLocalPath(QDir::homePath())
    , m_currentRemotePath("/")
    , m_currentDragAction(DragDropAction::None)
    , m_connectButton(nullptr)
    , m_disconnectButton(nullptr)
    , m_savedConnectionsList(nullptr)
    , m_localView(nullptr)
    , m_remoteView(nullptr)
    , m_localPathEdit(nullptr)
    , m_remotePathEdit(nullptr)
    , m_uploadButton(nullptr)
    , m_downloadButton(nullptr)
    , m_progressBar(nullptr)
    , m_statusLabel(nullptr)
    , m_logTextEdit(nullptr)
    , m_themeComboBox(nullptr)
    , m_connectionDialog(nullptr)
    , m_serverInput(nullptr)
    , m_usernameInput(nullptr)
    , m_passwordInput(nullptr)
    , m_portInput(nullptr)
    , m_connectionTypeCombo(nullptr)
{
    qDebug() << "MainWindow constructor started";
    
    try {
        ui->setupUi(this);
        setWindowTitle("DarkFTP");
        resize(1024, 768);
        
        // Skapa anslutningsdialog först
        setupConnectionDialog();
        
        // Ladda inställningar och anslutningar
        loadSettings();
        
        setupUi();
        setupModels();
        setupConnections();
        
        // FTP Manager-signaler
        connect(m_ftpManager, &FtpManager::error, this, &MainWindow::onFtpError);
        connect(m_ftpManager, &FtpManager::connected, this, &MainWindow::onFtpConnected);
        connect(m_ftpManager, &FtpManager::disconnected, this, &MainWindow::onFtpDisconnected);
        connect(m_ftpManager, &FtpManager::directoryListed, this, &MainWindow::onDirectoryListed);
        
        // Anpassa anslutningar för att hantera filsökvägar i signalerna
        connect(m_ftpManager, &FtpManager::downloadProgress, 
                this, [=](const QString &filePath, qint64 bytesReceived, qint64 bytesTotal) {
                    // Skicka vidare till onFtpDownloadProgress
                    onFtpDownloadProgress(filePath, bytesReceived, bytesTotal);
                });
        connect(m_ftpManager, &FtpManager::uploadProgress, 
                this, [=](const QString &filePath, qint64 bytesSent, qint64 bytesTotal) {
                    // Skicka vidare till onFtpUploadProgress
                    onFtpUploadProgress(filePath, bytesSent, bytesTotal);
                });
        
        connect(m_ftpManager, &FtpManager::downloadFinished, this, &MainWindow::onDownloadFinished);
        connect(m_ftpManager, &FtpManager::uploadFinished, this, &MainWindow::onUploadFinished);
        
        // SFTP Manager-signaler
        connect(m_sftpManager, &SftpManager::error, this, &MainWindow::onFtpError);
        connect(m_sftpManager, &SftpManager::connected, this, &MainWindow::onFtpConnected);
        connect(m_sftpManager, &SftpManager::disconnected, this, &MainWindow::onFtpDisconnected);
        connect(m_sftpManager, &SftpManager::directoryListed, this, &MainWindow::onDirectoryListed);
        
        // Anpassa anslutningar för att hantera filsökvägar i signalerna
        connect(m_sftpManager, &SftpManager::downloadProgress, 
                this, &MainWindow::onFtpDownloadProgress);
        connect(m_sftpManager, &SftpManager::uploadProgress, 
                this, &MainWindow::onFtpUploadProgress);
        
        connect(m_sftpManager, &SftpManager::downloadFinished, this, &MainWindow::onDownloadFinished);
        connect(m_sftpManager, &SftpManager::uploadFinished, this, &MainWindow::onUploadFinished);
        connect(m_sftpManager, &SftpManager::logMessage, this, &MainWindow::appendToLog);
        
        // Sätt standardtema
        setTheme(AppTheme::DarkTermius);
        
        // Logga uppstart
        appendToLog(tr("DarkFTP startad - redo att ansluta"));
        
        updateUIState();
        updateConnectionsList();
        
        qDebug() << "MainWindow constructor completed successfully";
    } catch (const std::exception& e) {
        qDebug() << "Exception in MainWindow constructor: " << e.what();
    } catch (...) {
        qDebug() << "Unknown exception in MainWindow constructor";
    }
}

MainWindow::~MainWindow()
{
    if (m_ftpManager->isConnected()) {
        m_ftpManager->disconnectFromServer();
    }
    
    // Spara inställningar
    saveSettings();
    
    delete ui;
}

void MainWindow::setTheme(AppTheme theme)
{
    QString styleSheet;
    
    switch (theme) {
        case AppTheme::DarkTermius:
            styleSheet = 
                "QMainWindow, QDialog { background-color: #1A1D21; color: #FFFFFF; }"
                "QTreeView, QListView, QTableView { background-color: #262626; color: #FFFFFF; alternate-background-color: #313131; }"
                "QTreeView::item:selected, QListView::item:selected, QTableView::item:selected { background-color: #3A539B; }"
                "QLineEdit, QTextEdit, QPlainTextEdit, QSpinBox { background-color: #333333; color: #FFFFFF; border: 1px solid #555555; }"
                "QPushButton { background-color: #3A539B; color: #FFFFFF; padding: 5px 10px; border-radius: 2px; }"
                "QPushButton:hover { background-color: #4A63B8; }"
                "QPushButton:pressed { background-color: #2A3A7B; }"
                "QPushButton:disabled { background-color: #555555; color: #888888; }"
                "QHeaderView::section { background-color: #333333; color: #FFFFFF; padding: 4px; }"
                "QTabWidget::pane { border: 1px solid #555555; }"
                "QTabBar::tab { background-color: #262626; color: #FFFFFF; padding: 8px 12px; margin-right: 1px; }"
                "QTabBar::tab:selected { background-color: #3A539B; }"
                "QTabBar::tab:hover:!selected { background-color: #333333; }"
                "QProgressBar { border: 1px solid #555555; background-color: #333333; color: #FFFFFF; text-align: center; }"
                "QProgressBar::chunk { background-color: #3A539B; }"
                "QComboBox { background-color: #333333; color: #FFFFFF; border: 1px solid #555555; padding: 4px 8px; }"
                "QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 15px; border-left-width: 1px; border-left-color: #555555; border-left-style: solid; }"
                "QComboBox QAbstractItemView { background-color: #333333; color: #FFFFFF; selection-background-color: #3A539B; }";
            break;
            
        case AppTheme::RetroBlue:
            styleSheet = 
                "QMainWindow, QDialog { background-color: #2B3A42; color: #EFEFEF; }"
                "QTreeView, QListView, QTableView { background-color: #3F5866; color: #EFEFEF; alternate-background-color: #355263; }"
                "QTreeView::item:selected, QListView::item:selected, QTableView::item:selected { background-color: #1E5B8F; }"
                "QLineEdit, QTextEdit, QPlainTextEdit, QSpinBox { background-color: #3F5866; color: #EFEFEF; border: 1px solid #4E6B79; }"
                "QPushButton { background-color: #1E5B8F; color: #EFEFEF; padding: 5px 10px; border-radius: 2px; }"
                "QPushButton:hover { background-color: #2E6BA0; }"
                "QPushButton:pressed { background-color: #0E4B7F; }"
                "QPushButton:disabled { background-color: #4E6B79; color: #8A9DA8; }"
                "QHeaderView::section { background-color: #405E6F; color: #EFEFEF; padding: 4px; }"
                "QTabWidget::pane { border: 1px solid #4E6B79; }"
                "QTabBar::tab { background-color: #3F5866; color: #EFEFEF; padding: 8px 12px; margin-right: 1px; }"
                "QTabBar::tab:selected { background-color: #1E5B8F; }"
                "QTabBar::tab:hover:!selected { background-color: #405E6F; }"
                "QProgressBar { border: 1px solid #4E6B79; background-color: #3F5866; color: #EFEFEF; text-align: center; }"
                "QProgressBar::chunk { background-color: #1E5B8F; }"
                "QComboBox { background-color: #3F5866; color: #EFEFEF; border: 1px solid #4E6B79; padding: 4px 8px; }"
                "QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 15px; border-left-width: 1px; border-left-color: #4E6B79; border-left-style: solid; }"
                "QComboBox QAbstractItemView { background-color: #3F5866; color: #EFEFEF; selection-background-color: #1E5B8F; }";
            break;

        case AppTheme::Steampunk:
            styleSheet = 
                "QMainWindow, QDialog { background-color: #2D2317; color: #E8D0AA; }"
                "QTreeView, QListView, QTableView { background-color: #3A2E1F; color: #E8D0AA; alternate-background-color: #483A26; border: 1px solid #73603B; }"
                "QTreeView::item:selected, QListView::item:selected, QTableView::item:selected { background-color: #784E29; color: #FFF7E8; }"
                "QLineEdit, QTextEdit, QPlainTextEdit, QSpinBox { background-color: #483A26; color: #E8D0AA; border: 2px solid #73603B; border-radius: 4px; padding: 2px; }"
                "QPushButton { background-color: #73603B; color: #E8D0AA; padding: 5px 10px; border: 2px solid #8F743D; border-radius: 6px; }"
                "QPushButton:hover { background-color: #8F743D; border-color: #A88D51; }"
                "QPushButton:pressed { background-color: #5E4E32; }"
                "QPushButton:disabled { background-color: #3A2E1F; color: #73603B; border-color: #483A26; }"
                "QHeaderView::section { background-color: #73603B; color: #E8D0AA; padding: 4px; }"
                "QTabWidget::pane { border: 2px solid #73603B; }"
                "QTabBar::tab { background-color: #483A26; color: #E8D0AA; padding: 8px 12px; margin-right: 1px; border: 1px solid #73603B; border-bottom: none; border-top-left-radius: 4px; border-top-right-radius: 4px; }"
                "QTabBar::tab:selected { background-color: #73603B; }"
                "QTabBar::tab:hover:!selected { background-color: #5E4E32; }"
                "QProgressBar { border: 2px solid #73603B; background-color: #3A2E1F; color: #E8D0AA; text-align: center; border-radius: 4px; }"
                "QProgressBar::chunk { background-color: #73603B; }"
                "QComboBox { background-color: #483A26; color: #E8D0AA; border: 2px solid #73603B; padding: 4px 8px; border-radius: 4px; }"
                "QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 20px; border-left-width: 2px; border-left-color: #73603B; border-left-style: solid; }"
                "QComboBox QAbstractItemView { background-color: #483A26; color: #E8D0AA; selection-background-color: #73603B; }"
                "QMenuBar { background-color: #3A2E1F; color: #E8D0AA; border-bottom: 2px solid #73603B; }"
                "QMenuBar::item { background: transparent; padding: 6px 10px; }"
                "QMenuBar::item:selected { background-color: #73603B; }"
                "QMenu { background-color: #3A2E1F; color: #E8D0AA; border: 1px solid #73603B; }"
                "QMenu::item:selected { background-color: #73603B; }";
            break;

        case AppTheme::Hacker:
            styleSheet = 
                "QMainWindow, QDialog { background-color: #0A0A0A; color: #33FF33; }"
                "QTreeView, QListView, QTableView { background-color: #0F0F0F; color: #33FF33; alternate-background-color: #151515; border: 1px solid #333333; }"
                "QTreeView::item:selected, QListView::item:selected, QTableView::item:selected { background-color: #002200; color: #66FF66; }"
                "QLineEdit, QTextEdit, QPlainTextEdit, QSpinBox { background-color: #0F0F0F; color: #33FF33; border: 1px solid #33FF33; }"
                "QPushButton { background-color: #0F0F0F; color: #33FF33; padding: 5px 10px; border: 1px solid #33FF33; }"
                "QPushButton:hover { border-width: 2px; }"
                "QPushButton:pressed { background-color: #002200; }"
                "QPushButton:disabled { color: #006600; border-color: #006600; }"
                "QHeaderView::section { background-color: #0F0F0F; color: #33FF33; padding: 4px; border: 1px solid #33FF33; }"
                "QTabWidget::pane { border: 1px solid #33FF33; }"
                "QTabBar::tab { background-color: #0F0F0F; color: #33FF33; padding: 8px 12px; margin-right: 1px; border: 1px solid #33FF33; }"
                "QTabBar::tab:selected { background-color: #002200; }"
                "QTabBar::tab:hover:!selected { background-color: #151515; }"
                "QProgressBar { border: 1px solid #33FF33; background-color: #0F0F0F; color: #33FF33; text-align: center; }"
                "QProgressBar::chunk { background-color: #007700; }"
                "QComboBox { background-color: #0F0F0F; color: #33FF33; border: 1px solid #33FF33; padding: 4px 8px; }"
                "QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 15px; border-left-width: 1px; border-left-color: #33FF33; border-left-style: solid; }"
                "QComboBox QAbstractItemView { background-color: #0F0F0F; color: #33FF33; selection-background-color: #002200; }"
                "QMenuBar { background-color: #0A0A0A; color: #33FF33; }"
                "QMenuBar::item { background: transparent; padding: 6px 10px; }"
                "QMenuBar::item:selected { background-color: #002200; }"
                "QMenu { background-color: #0F0F0F; color: #33FF33; border: 1px solid #33FF33; }"
                "QMenu::item:selected { background-color: #002200; }"
                "QScrollBar:vertical { border: 1px solid #33FF33; background: #0F0F0F; width: 12px; margin: 12px 0 12px 0; }"
                "QScrollBar::handle:vertical { background: #007700; min-height: 20px; }"
                "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { border: 1px solid #33FF33; background: #0F0F0F; height: 12px; subcontrol-position: top; subcontrol-origin: margin; }";
            break;

        case AppTheme::Nordisk:
            styleSheet = 
                "QMainWindow, QDialog { background-color: #2E3440; color: #D8DEE9; }"
                "QTreeView, QListView, QTableView { background-color: #3B4252; color: #E5E9F0; alternate-background-color: #434C5E; }"
                "QTreeView::item:selected, QListView::item:selected, QTableView::item:selected { background-color: #5E81AC; color: #ECEFF4; }"
                "QLineEdit, QTextEdit, QPlainTextEdit, QSpinBox { background-color: #3B4252; color: #E5E9F0; border: 1px solid #4C566A; }"
                "QPushButton { background-color: #5E81AC; color: #ECEFF4; padding: 5px 10px; border-radius: 2px; }"
                "QPushButton:hover { background-color: #81A1C1; }"
                "QPushButton:pressed { background-color: #4C566A; }"
                "QPushButton:disabled { background-color: #434C5E; color: #4C566A; }"
                "QHeaderView::section { background-color: #4C566A; color: #E5E9F0; padding: 4px; }"
                "QTabWidget::pane { border: 1px solid #4C566A; }"
                "QTabBar::tab { background-color: #3B4252; color: #E5E9F0; padding: 8px 12px; margin-right: 1px; }"
                "QTabBar::tab:selected { background-color: #5E81AC; color: #ECEFF4; }"
                "QTabBar::tab:hover:!selected { background-color: #434C5E; }"
                "QProgressBar { border: 1px solid #4C566A; background-color: #3B4252; color: #E5E9F0; text-align: center; }"
                "QProgressBar::chunk { background-color: #5E81AC; }"
                "QComboBox { background-color: #3B4252; color: #E5E9F0; border: 1px solid #4C566A; padding: 4px 8px; }"
                "QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 15px; border-left-width: 1px; border-left-color: #4C566A; border-left-style: solid; }"
                "QComboBox QAbstractItemView { background-color: #3B4252; color: #E5E9F0; selection-background-color: #5E81AC; }"
                "QMenuBar { background-color: #2E3440; color: #D8DEE9; }"
                "QMenuBar::item { background: transparent; padding: 6px 10px; }"
                "QMenuBar::item:selected { background-color: #3B4252; }"
                "QMenu { background-color: #3B4252; color: #E5E9F0; border: 1px solid #4C566A; }"
                "QMenu::item:selected { background-color: #5E81AC; }";
            break;
    }
    
    qApp->setStyleSheet(styleSheet);
}

void MainWindow::switchTheme(int themeIndex)
{
    AppTheme theme;
    
    switch (themeIndex) {
        case 0:
            theme = AppTheme::DarkTermius;
            break;
        case 1:
            theme = AppTheme::RetroBlue;
            break;
        case 2:
            theme = AppTheme::Steampunk;
            break;
        case 3:
            theme = AppTheme::Hacker;
            break;
        case 4:
            theme = AppTheme::Nordisk;
            break;
        default:
            theme = AppTheme::DarkTermius;
            break;
    }
    
    setTheme(theme);
}

void MainWindow::loadSettings()
{
    QSettings settings("DarkFTP", "Settings");
    
    // Läs in tema
    int themeIndex = settings.value("theme", 0).toInt();
    AppTheme theme = static_cast<AppTheme>(themeIndex);
    setTheme(theme);
    
    // Läs in anslutningar
    int count = settings.beginReadArray("connections");
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        
        ConnectionInfo info;
        info.name = settings.value("name").toString();
        info.type = static_cast<ConnectionType>(settings.value("type").toInt());
        info.host = settings.value("host").toString();
        info.port = settings.value("port").toInt();
        info.username = settings.value("username").toString();
        info.password = settings.value("password").toString();
        
        m_savedConnections.append(info);
    }
    settings.endArray();
    
    updateConnectionsList();
}

void MainWindow::saveSettings()
{
    QSettings settings("DarkFTP", "Settings");
    
    // Spara tema
    // Kommentera bort raden som hänvisar till m_themeComboBox
    //settings.setValue("theme", m_themeComboBox->currentIndex());
    
    // Spara anslutningar
    settings.beginWriteArray("connections");
    for (int i = 0; i < m_savedConnections.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("name", m_savedConnections[i].name);
        settings.setValue("type", static_cast<int>(m_savedConnections[i].type));
        settings.setValue("host", m_savedConnections[i].host);
        settings.setValue("port", m_savedConnections[i].port);
        settings.setValue("username", m_savedConnections[i].username);
        // Spara inte lösenord i klartext i en riktig app
        settings.setValue("password", m_savedConnections[i].password);
    }
    settings.endArray();
}

void MainWindow::setupUi()
{
    qDebug() << "Setting up UI";
    
    // Skapa huvud-splitter och layouter
    QSplitter *mainSplitter = new QSplitter(Qt::Vertical, this);
    QSplitter *fileSplitter = new QSplitter(Qt::Horizontal, this);
    
    // Skapa widgets för filhantering
    QWidget *localContainer = new QWidget();
    QVBoxLayout *localLayout = new QVBoxLayout(localContainer);
    m_localView = new QTreeView();
    m_localPathEdit = new QLineEdit();
    QPushButton *browseButton = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Bläddra..."));
    QHBoxLayout *localPathLayout = new QHBoxLayout();
    
    QLabel *homeIconLabel = new QLabel();
    homeIconLabel->setPixmap(QApplication::style()->standardIcon(QStyle::SP_DirHomeIcon).pixmap(16, 16));
    localPathLayout->addWidget(homeIconLabel);
    
    localPathLayout->addWidget(m_localPathEdit);
    localPathLayout->addWidget(browseButton);
    QLabel *localLabel = new QLabel(tr("Lokala filer"));
    localLabel->setStyleSheet("font-weight: bold; font-size: 11pt; padding: 4px;");
    localLayout->addWidget(localLabel);
    localLayout->addLayout(localPathLayout);
    localLayout->addWidget(m_localView);
    
    // Rundade hörn för lokala fil-containern
    localContainer->setStyleSheet("background-color: rgba(40, 40, 40, 50); border-radius: 8px; margin: 2px;");
    
    QWidget *remoteContainer = new QWidget();
    QVBoxLayout *remoteLayout = new QVBoxLayout(remoteContainer);
    m_remoteView = new QTreeView();
    m_remotePathEdit = new QLineEdit();
    QPushButton *goButton = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton), tr("Gå"));
    QHBoxLayout *remotePathLayout = new QHBoxLayout();
    
    QLabel *netIconLabel = new QLabel();
    netIconLabel->setPixmap(QApplication::style()->standardIcon(QStyle::SP_DriveNetIcon).pixmap(16, 16));
    remotePathLayout->addWidget(netIconLabel);
    
    remotePathLayout->addWidget(m_remotePathEdit);
    remotePathLayout->addWidget(goButton);
    QLabel *remoteLabel = new QLabel(tr("Fjärrfiler"));
    remoteLabel->setStyleSheet("font-weight: bold; font-size: 11pt; padding: 4px;");
    remoteLayout->addWidget(remoteLabel);
    remoteLayout->addLayout(remotePathLayout);
    remoteLayout->addWidget(m_remoteView);
    
    // Rundade hörn för fjärr-containern
    remoteContainer->setStyleSheet("background-color: rgba(40, 40, 40, 50); border-radius: 8px; margin: 2px;");
    
    fileSplitter->addWidget(localContainer);
    fileSplitter->addWidget(remoteContainer);
    fileSplitter->setStretchFactor(0, 1);
    fileSplitter->setStretchFactor(1, 1);
    
    // Skapa transferwidget med progressbar
    QWidget *transferWidget = new QWidget();
    QVBoxLayout *transferLayout = new QVBoxLayout(transferWidget);
    m_progressBar = new QProgressBar();
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    m_progressBar->setValue(0);
    m_statusLabel = new QLabel(tr("Redo"));
    
    // Lägg till kontrollknappar med ikoner
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_uploadButton = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_ArrowUp), tr("Ladda upp"));
    m_downloadButton = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_ArrowDown), tr("Ladda ner"));
    QPushButton *refreshButton = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_BrowserReload), tr("Uppdatera"));
    buttonLayout->addWidget(m_uploadButton);
    buttonLayout->addWidget(m_downloadButton);
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();
    
    transferLayout->addLayout(buttonLayout);
    transferLayout->addWidget(m_statusLabel);
    transferLayout->addWidget(m_progressBar);
    
    // Skapa loggfönster
    QWidget *logContainer = new QWidget();
    QVBoxLayout *logLayout = new QVBoxLayout(logContainer);
    m_logTextEdit = new QTextEdit();
    m_logTextEdit->setReadOnly(true);
    // Ge loggfönstret ett monospace-typsnitt
    QFont logFont("Courier New", 9);
    m_logTextEdit->setFont(logFont);
    QPushButton *clearLogButton = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_DialogDiscardButton), tr("Rensa logg"));
    
    QLabel *logLabel = new QLabel(tr("Aktivitetslogg"));
    logLabel->setStyleSheet("font-weight: bold; font-size: 11pt; padding: 4px;");
    
    QHBoxLayout *logControlLayout = new QHBoxLayout();
    logControlLayout->addWidget(logLabel);
    logControlLayout->addStretch();
    logControlLayout->addWidget(clearLogButton);
    
    logLayout->addLayout(logControlLayout);
    logLayout->addWidget(m_logTextEdit);
    
    // Anslut knappar till funktioner
    connect(browseButton, &QPushButton::clicked, this, &MainWindow::browseLocalDirectory);
    connect(goButton, &QPushButton::clicked, this, &MainWindow::updateRemoteDirectory);
    connect(m_uploadButton, &QPushButton::clicked, this, &MainWindow::uploadFile);
    connect(m_downloadButton, &QPushButton::clicked, this, &MainWindow::downloadFile);
    connect(refreshButton, &QPushButton::clicked, this, [this]() {
        updateRemoteDirectory();
    });
    connect(clearLogButton, &QPushButton::clicked, this, &MainWindow::clearLog);
    
    // Lägg till alla komponenter i huvudlayouten
    mainSplitter->addWidget(fileSplitter);
    
    // Lägg till en tab-widget för transferinfo och logg
    QTabWidget *bottomTabWidget = new QTabWidget();
    bottomTabWidget->addTab(transferWidget, QApplication::style()->standardIcon(QStyle::SP_FileDialogContentsView), tr("Överföringar"));
    bottomTabWidget->addTab(logContainer, QApplication::style()->standardIcon(QStyle::SP_FileDialogInfoView), tr("Loggfönster"));
    
    mainSplitter->addWidget(bottomTabWidget);
    mainSplitter->setStretchFactor(0, 3);
    mainSplitter->setStretchFactor(1, 1);
    
    // Inaktivera knappar initiellt (innan anslutning)
    m_uploadButton->setEnabled(false);
    m_downloadButton->setEnabled(false);
    
    setCentralWidget(mainSplitter);
    
    // Skapa menyer och verktygsfält
    createMenus();
    
    // Ställ in fönsterstorlek och position
    setMinimumSize(900, 600);
    resize(1200, 800);
    
    // För att centrera fönstret på skärmen
    setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            size(),
            QGuiApplication::primaryScreen()->availableGeometry()
        )
    );
}

void MainWindow::setupConnectionDialog()
{
    m_connectionDialog = new QDialog(this);
    m_connectionDialog->setWindowTitle(tr("Anslut till server"));
    m_connectionDialog->setMinimumWidth(500);
    m_connectionDialog->setWindowIcon(QApplication::style()->standardIcon(QStyle::SP_DriveNetIcon));
    
    QVBoxLayout *layout = new QVBoxLayout(m_connectionDialog);
    
    // Header med titel och ikon
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *iconLabel = new QLabel();
    iconLabel->setPixmap(QApplication::style()->standardIcon(QStyle::SP_DriveNetIcon).pixmap(32, 32));
    QLabel *titleLabel = new QLabel(tr("Anslut till FTP/SFTP-server"));
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #4A86E8;");
    headerLayout->addWidget(iconLabel);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    layout->addLayout(headerLayout);
    
    // Separator
    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    layout->addWidget(separator);
    layout->addSpacing(10);
    
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(10);
    formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    
    // Server med ikon
    QHBoxLayout *serverLayout = new QHBoxLayout();
    QLabel *serverIconLabel = new QLabel();
    serverIconLabel->setPixmap(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon).pixmap(16, 16));
    m_serverInput = new QLineEdit(m_connectionDialog);
    m_serverInput->setPlaceholderText("ftp.example.com");
    serverLayout->addWidget(serverIconLabel);
    serverLayout->addWidget(m_serverInput);
    formLayout->addRow(tr("Server:"), serverLayout);
    
    // Användarnamn med ikon
    QHBoxLayout *userLayout = new QHBoxLayout();
    QLabel *userIconLabel = new QLabel();
    userIconLabel->setPixmap(QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton).pixmap(16, 16));
    m_usernameInput = new QLineEdit(m_connectionDialog);
    m_usernameInput->setPlaceholderText(tr("Användarnamn"));
    userLayout->addWidget(userIconLabel);
    userLayout->addWidget(m_usernameInput);
    formLayout->addRow(tr("Användarnamn:"), userLayout);
    
    // Lösenord med ikon
    QHBoxLayout *passLayout = new QHBoxLayout();
    QLabel *passIconLabel = new QLabel();
    passIconLabel->setPixmap(QApplication::style()->standardIcon(QStyle::SP_DialogNoButton).pixmap(16, 16));
    m_passwordInput = new QLineEdit(m_connectionDialog);
    m_passwordInput->setPlaceholderText(tr("Lösenord"));
    m_passwordInput->setEchoMode(QLineEdit::Password);
    passLayout->addWidget(passIconLabel);
    passLayout->addWidget(m_passwordInput);
    formLayout->addRow(tr("Lösenord:"), passLayout);
    
    // Port med ikon
    QHBoxLayout *portLayout = new QHBoxLayout();
    QLabel *portIconLabel = new QLabel();
    portIconLabel->setPixmap(QApplication::style()->standardIcon(QStyle::SP_MediaPlay).pixmap(16, 16));
    m_portInput = new QLineEdit(m_connectionDialog);
    m_portInput->setPlaceholderText("21");
    // Begränsa inmatningen till siffror
    QIntValidator *validator = new QIntValidator(1, 65535, m_connectionDialog);
    m_portInput->setValidator(validator);
    portLayout->addWidget(portIconLabel);
    portLayout->addWidget(m_portInput);
    formLayout->addRow(tr("Port:"), portLayout);
    
    // Anslutningstyp med ikon
    QHBoxLayout *typeLayout = new QHBoxLayout();
    QLabel *typeIconLabel = new QLabel();
    typeIconLabel->setPixmap(QApplication::style()->standardIcon(QStyle::SP_FileDialogListView).pixmap(16, 16));
    m_connectionTypeCombo = new QComboBox(m_connectionDialog);
    m_connectionTypeCombo->addItem("FTP");
    m_connectionTypeCombo->addItem("SFTP");
    typeLayout->addWidget(typeIconLabel);
    typeLayout->addWidget(m_connectionTypeCombo);
    formLayout->addRow(tr("Anslutningstyp:"), typeLayout);
    
    // Lägg till lite extra avstånd för bättre utseende
    formLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    
    layout->addLayout(formLayout);
    
    // Uppdatera port när anslutningstyp ändras
    connect(m_connectionTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (index == 0) { // FTP
            m_portInput->setText("21");
        } else if (index == 1) { // SFTP
            m_portInput->setText("22");
        }
    });
    
    // Spara anslutning checkbox
    QCheckBox *saveConnectionCheckBox = new QCheckBox(tr("Spara anslutning för framtida användning"));
    layout->addWidget(saveConnectionCheckBox);
    
    // Lite mellanrum före knapparna
    layout->addSpacing(20);
    
    // Knappar med stöd för att spara anslutning
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
    
    // Använd redan skapad m_connectButton eller skapa en ny om den inte finns
    if (!m_connectButton) {
        m_connectButton = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_CommandLink), tr("Anslut"), m_connectionDialog);
        m_connectButton->setStyleSheet("padding: 6px 12px;");
    }
    buttonBox->addButton(m_connectButton, QDialogButtonBox::AcceptRole);
    
    QPushButton *cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
    cancelButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
    cancelButton->setText(tr("Avbryt"));
    cancelButton->setStyleSheet("padding: 6px 12px;");
    
    layout->addWidget(buttonBox);
    
    // Anslut knapparna
    connect(buttonBox, &QDialogButtonBox::accepted, m_connectionDialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, m_connectionDialog, &QDialog::reject);
    connect(m_connectionDialog, &QDialog::accepted, [this, saveConnectionCheckBox]() {
        // Spara anslutningen om checkboxen är markerad
        if (saveConnectionCheckBox->isChecked()) {
            saveConnection();
        }
        connectToFtp();
    });
    
    // Sätt fokus på serverinmatning när dialogen öppnas
    connect(m_connectionDialog, &QDialog::finished, [this](int) {
        m_serverInput->setFocus();
    });
}

void MainWindow::showConnectionDialog()
{
    // Återställ fält till standardvärden
    m_serverInput->clear();
    m_usernameInput->clear();
    m_passwordInput->clear();
    m_connectionTypeCombo->setCurrentIndex(0); // FTP
    m_portInput->setText("21");
    
    // Visa dialogrutan
    if (m_connectionDialog->exec() == QDialog::Accepted) {
        // Användaren klickade på OK/Anslut, connectToFtp anropas automatiskt
    }
}

void MainWindow::updateConnectionsList()
{
    // Kontrollera om m_savedConnectionsList är initierat
    if (!m_savedConnectionsList) {
        // Vi fortsätter utan att göra något, det är inte kritiskt
        return;
    }
    
    m_savedConnectionsList->clear();
    
    for (const ConnectionInfo &info : m_savedConnections) {
        QString displayText = info.name.isEmpty() ? info.host : info.name;
        QString typeText = (info.type == ConnectionType::FTP) ? "FTP" : "SFTP";
        displayText = QString("%1 (%2://%3@%4:%5)").arg(
            displayText, typeText, info.username, info.host, QString::number(info.port));
        
        QListWidgetItem *item = new QListWidgetItem(displayText);
        m_savedConnectionsList->addItem(item);
    }
}

void MainWindow::saveConnection()
{
    // Kontrollera om anslutningen redan finns
    int existingIndex = -1;
    for (int i = 0; i < m_savedConnections.size(); ++i) {
        if (m_savedConnections[i] == m_currentConnection) {
            existingIndex = i;
            break;
        }
    }
    
    // Om vi inte har ett namn, använd server som namn
    if (m_currentConnection.name.isEmpty()) {
        m_currentConnection.name = m_currentConnection.host;
    }
    
    if (existingIndex >= 0) {
        // Uppdatera befintlig anslutning
        m_savedConnections[existingIndex] = m_currentConnection;
    } else {
        // Lägg till ny anslutning
        m_savedConnections.append(m_currentConnection);
    }
    
    updateConnectionsList();
    saveSettings();
}

void MainWindow::loadConnection(int index)
{
    if (index >= 0 && index < m_savedConnections.size()) {
        m_currentConnection = m_savedConnections[index];
        
        m_serverInput->setText(m_currentConnection.host);
        m_usernameInput->setText(m_currentConnection.username);
        m_passwordInput->setText(m_currentConnection.password);
        m_portInput->setText(QString::number(m_currentConnection.port));
        m_activeConnectionType = m_currentConnection.type;
    }
}

void MainWindow::showSavedConnectionsDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Hantera sparade anslutningar"));
    dialog.setMinimumWidth(600);
    dialog.setMinimumHeight(450);
    
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    
    // Titelrad med stilisering
    QLabel *titleLabel = new QLabel(tr("Sparade anslutningar"));
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #4A86E8; padding: 10px 0;");
    layout->addWidget(titleLabel);
    
    // Skapa listwidget för att visa sparade anslutningar
    QListWidget *connectionsList = new QListWidget(&dialog);
    connectionsList->setAlternatingRowColors(true);
    connectionsList->setSelectionMode(QListWidget::SingleSelection);
    
    // Container för anslutningslistan med ram
    QWidget *listContainer = new QWidget(&dialog);
    QVBoxLayout *listLayout = new QVBoxLayout(listContainer);
    listLayout->setContentsMargins(0, 0, 0, 0);
    listLayout->addWidget(connectionsList);
    listContainer->setStyleSheet("border: 1px solid #555555; border-radius: 4px; padding: 2px;");
    
    layout->addWidget(listContainer);
    
    // Uppdatera listan med sparade anslutningar
    int rowCounter = 0;
    for (const ConnectionInfo &info : m_savedConnections) {
        QString displayText = info.name.isEmpty() ? info.host : info.name;
        QString typeText = (info.type == ConnectionType::FTP) ? "FTP" : "SFTP";
        
        QListWidgetItem *item = new QListWidgetItem();
        
        // Skapa en widget för att visa anslutningsinformation med ikon
        QWidget *connectionWidget = new QWidget();
        QHBoxLayout *connectionLayout = new QHBoxLayout(connectionWidget);
        
        // Välj ikon baserat på anslutningstyp
        QIcon icon = (info.type == ConnectionType::FTP) 
                    ? QApplication::style()->standardIcon(QStyle::SP_DriveNetIcon)
                    : QApplication::style()->standardIcon(QStyle::SP_DriveNetIcon);
        
        QLabel *iconLabel = new QLabel();
        iconLabel->setPixmap(icon.pixmap(24, 24));
        connectionLayout->addWidget(iconLabel);
        
        // Information om anslutningen
        QVBoxLayout *infoLayout = new QVBoxLayout();
        QLabel *nameLabel = new QLabel(displayText);
        nameLabel->setStyleSheet("font-weight: bold;");
        QLabel *detailsLabel = new QLabel(QString("%1://%2@%3:%4").arg(
            typeText, info.username, info.host, QString::number(info.port)));
        detailsLabel->setStyleSheet("color: #888888;");
        
        infoLayout->addWidget(nameLabel);
        infoLayout->addWidget(detailsLabel);
        connectionLayout->addLayout(infoLayout);
        connectionLayout->addStretch();
        
        // Sätt widgeten i listvyns objekt
        connectionsList->addItem(item);
        item->setSizeHint(connectionWidget->sizeHint() + QSize(20, 20)); // Lägg till lite extra utrymme
        connectionsList->setItemWidget(item, connectionWidget);
        
        rowCounter++;
    }
    
    // Informationstext om det inte finns några anslutningar
    if (rowCounter == 0) {
        QLabel *noConnectionsLabel = new QLabel(tr("Inga sparade anslutningar. Använd \"Spara anslutning\" för att lägga till."));
        noConnectionsLabel->setAlignment(Qt::AlignCenter);
        noConnectionsLabel->setStyleSheet("color: #888888; padding: 20px;");
        connectionsList->addItem("");
        QListWidgetItem *item = connectionsList->item(0);
        connectionsList->setItemWidget(item, noConnectionsLabel);
    }
    
    // Knappar för att hantera anslutningar
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *connectButton = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_CommandLink), tr("Anslut"), &dialog);
    QPushButton *editButton = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_FileDialogDetailedView), tr("Redigera"), &dialog);
    QPushButton *deleteButton = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_TrashIcon), tr("Ta bort"), &dialog);
    QPushButton *closeButton = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton), tr("Stäng"), &dialog);
    
    connectButton->setStyleSheet("padding: 6px 12px;");
    editButton->setStyleSheet("padding: 6px 12px;");
    deleteButton->setStyleSheet("padding: 6px 12px;");
    closeButton->setStyleSheet("padding: 6px 12px;");
    
    buttonLayout->addWidget(connectButton);
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    
    layout->addLayout(buttonLayout);
    
    // Aktivera/inaktivera knappar baserat på val
    auto updateButtons = [&]() {
        bool hasSelection = connectionsList->currentRow() >= 0;
        connectButton->setEnabled(hasSelection);
        editButton->setEnabled(hasSelection);
        deleteButton->setEnabled(hasSelection);
    };
    
    connect(connectionsList, &QListWidget::currentRowChanged, updateButtons);
    updateButtons(); // Initialt tillstånd
    
    // Anslut knapparna
    connect(connectButton, &QPushButton::clicked, [&]() {
        int currentRow = connectionsList->currentRow();
        if (currentRow >= 0 && currentRow < m_savedConnections.size()) {
            loadConnection(currentRow);
            dialog.accept();
            connectToFtp();
        }
    });
    
    connect(editButton, &QPushButton::clicked, [&]() {
        int currentRow = connectionsList->currentRow();
        if (currentRow >= 0 && currentRow < m_savedConnections.size()) {
            loadConnection(currentRow);
            dialog.accept();
            showConnectionDialog();
        }
    });
    
    connect(deleteButton, &QPushButton::clicked, [&]() {
        int currentRow = connectionsList->currentRow();
        if (currentRow >= 0 && currentRow < m_savedConnections.size()) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(&dialog, tr("Ta bort anslutning"), 
                                          tr("Är du säker på att du vill ta bort anslutningen '%1'?")
                                          .arg(m_savedConnections[currentRow].name),
                                          QMessageBox::Yes | QMessageBox::No);
                                          
            if (reply == QMessageBox::Yes) {
                m_savedConnections.removeAt(currentRow);
                delete connectionsList->takeItem(currentRow);
                saveSettings();
                updateButtons();
            }
        }
    });
    
    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    
    // Dubbelklick på en anslutning för att ansluta
    connect(connectionsList, &QListWidget::doubleClicked, [&](const QModelIndex &index) {
        int currentRow = index.row();
        if (currentRow >= 0 && currentRow < m_savedConnections.size()) {
            loadConnection(currentRow);
            dialog.accept();
            connectToFtp();
        }
    });
    
    dialog.exec();
}

void MainWindow::deleteConnection()
{
    int currentRow = m_savedConnectionsList->currentRow();
    if (currentRow >= 0 && currentRow < m_savedConnections.size()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Ta bort anslutning"), 
                                      tr("Är du säker på att du vill ta bort anslutningen '%1'?")
                                      .arg(m_savedConnections[currentRow].name),
                                      QMessageBox::Yes | QMessageBox::No);
                                      
        if (reply == QMessageBox::Yes) {
            m_savedConnections.removeAt(currentRow);
            updateConnectionsList();
            saveSettings();
        }
    }
}

void MainWindow::setupModels()
{
    try {
        qDebug() << "Setting up models";
        // Setup local file model
        m_localFileModel = new QFileSystemModel(this);
        m_localFileModel->setRootPath(QDir::homePath());
        
        m_localView->setModel(m_localFileModel);
        m_localView->setRootIndex(m_localFileModel->index(QDir::homePath()));
        m_localView->setSortingEnabled(true);
        m_localView->setAlternatingRowColors(true);
        m_localView->setAnimated(true);
        m_localView->setIndentation(20);
        m_localView->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_localView->setFont(QFont("Segoe UI", 10));
        
        // Aktivera drag och drop för lokalvyn
        m_localView->setDragEnabled(true);
        m_localView->setAcceptDrops(true);
        m_localView->setDropIndicatorShown(true);
        m_localView->setDragDropMode(QAbstractItemView::DragDrop);
        m_localView->installEventFilter(this);
        
        // Setup remote file model
        m_remoteFileModel = new QStandardItemModel(0, 3, this);
        m_remoteFileModel->setHorizontalHeaderLabels(QStringList() << tr("Namn") << tr("Storlek") << tr("Typ"));
        
        m_remoteView->setModel(m_remoteFileModel);
        m_remoteView->setSortingEnabled(true);
        m_remoteView->setAlternatingRowColors(true);
        m_remoteView->setFont(QFont("Segoe UI", 10));
        
        // Aktivera drag och drop för fjärrvyn
        m_remoteView->setDragEnabled(true);
        m_remoteView->setAcceptDrops(true);
        m_remoteView->setDropIndicatorShown(true);
        m_remoteView->setDragDropMode(QAbstractItemView::DragDrop);
        m_remoteView->installEventFilter(this);
        
        // Adjust column widths
        m_localView->setColumnWidth(0, 200);
        m_remoteView->setColumnWidth(0, 200);
        m_remoteView->setColumnWidth(1, 100);
        
        // Accept drag and drop on main window
        setAcceptDrops(true);
        
        m_currentLocalPath = QDir::homePath();
        m_localPathEdit->setText(m_currentLocalPath);
        
        // Connect signals
        connect(m_localView, &QTreeView::doubleClicked, this, &MainWindow::onLocalDirectorySelected);
        connect(m_remoteView, &QTreeView::doubleClicked, this, &MainWindow::onRemoteDirectorySelected);
        
        qDebug() << "Models setup complete";
    } catch (const std::exception& e) {
        qDebug() << "Error in setupModels: " << e.what();
    } catch (...) {
        qDebug() << "Unknown error in setupModels";
    }
}

void MainWindow::setupConnections()
{
    // Anslutningsknappar
    connect(m_connectButton, &QPushButton::clicked, this, &MainWindow::connectToFtp);
    connect(m_disconnectButton, &QPushButton::clicked, this, &MainWindow::disconnectFromFtp);
    
    // Lokala filsystem
    connect(m_localPathEdit, &QLineEdit::returnPressed, this, &MainWindow::updateRemoteDirectory);
    connect(m_localView, &QTreeView::doubleClicked, this, &MainWindow::onLocalDirectorySelected);
    
    // Fjärrfilsystem
    connect(m_remotePathEdit, &QLineEdit::returnPressed, this, &MainWindow::updateRemoteDirectory);
    connect(m_remoteView, &QTreeView::doubleClicked, this, &MainWindow::onRemoteDirectorySelected);
    
    // Överföringsknappar
    connect(m_uploadButton, &QPushButton::clicked, this, &MainWindow::uploadFile);
    connect(m_downloadButton, &QPushButton::clicked, this, &MainWindow::downloadFile);
    
    // FTP-hanterare
    connect(m_ftpManager, &FtpManager::connected, this, &MainWindow::onFtpConnected);
    connect(m_ftpManager, &FtpManager::disconnected, this, &MainWindow::onFtpDisconnected);
    connect(m_ftpManager, &FtpManager::error, this, &MainWindow::onFtpError);
    connect(m_ftpManager, &FtpManager::directoryListed, this, &MainWindow::onDirectoryListed);
    
    // SFTP Manager-signaler
    connect(m_sftpManager, &SftpManager::connected, this, &MainWindow::onFtpConnected);
    connect(m_sftpManager, &SftpManager::disconnected, this, &MainWindow::onFtpDisconnected);
    connect(m_sftpManager, &SftpManager::error, this, &MainWindow::onFtpError);
    connect(m_sftpManager, &SftpManager::directoryListed, this, &MainWindow::onDirectoryListed);
    
    // Anpassa anslutningar för att hantera filsökvägar i signalerna
    connect(m_sftpManager, &SftpManager::downloadProgress, 
            this, &MainWindow::onFtpDownloadProgress);
    connect(m_sftpManager, &SftpManager::uploadProgress, 
            this, &MainWindow::onFtpUploadProgress);
    
    connect(m_sftpManager, &SftpManager::downloadFinished, this, &MainWindow::onDownloadFinished);
    connect(m_sftpManager, &SftpManager::uploadFinished, this, &MainWindow::onUploadFinished);
    connect(m_sftpManager, &SftpManager::logMessage, this, &MainWindow::appendToLog);
}

void MainWindow::connectToFtp()
{
    m_statusLabel->setText(tr("Ansluter..."));
    m_progressBar->setValue(0);
    
    bool success = false;
    
    // Avläs inmatningsfälten från dialogen
    QString server = m_serverInput->text();
    int port = m_portInput->text().toInt();
    QString username = m_usernameInput->text();
    QString password = m_passwordInput->text();
    
    // Spara aktuell anslutningsinformation
    m_currentConnection.host = server;
    m_currentConnection.port = port;
    m_currentConnection.username = username;
    m_currentConnection.password = password;
    m_currentConnection.type = static_cast<ConnectionType>(m_connectionTypeCombo->currentIndex());
    
    m_activeConnectionType = m_currentConnection.type;
    
    // Anslut baserat på vald anslutningstyp
    if (m_activeConnectionType == ConnectionType::FTP) {
        appendToLog(tr("Ansluter till FTP-server: %1:%2").arg(server).arg(port));
        success = m_ftpManager->connectToServer(server, port, username, password);
    } else if (m_activeConnectionType == ConnectionType::SFTP) {
        appendToLog(tr("Ansluter till SFTP-server: %1:%2").arg(server).arg(port));
        success = m_sftpManager->connectToServer(server, port, username, password);
    }
    
    if (!success) {
        m_statusLabel->setText(tr("Anslutning misslyckades"));
        return;
    }
}

void MainWindow::disconnectFromFtp()
{
    if (m_activeConnectionType == ConnectionType::FTP) {
        m_ftpManager->disconnectFromServer();
    } else if (m_activeConnectionType == ConnectionType::SFTP) {
        m_sftpManager->disconnectFromServer();
    }
    
    m_uploadButton->setEnabled(false);
    m_downloadButton->setEnabled(false);
    m_statusLabel->setText(tr("Frånkopplad"));
    m_progressBar->setValue(0);
    
    // Rensa fjärrmodellen
    m_remoteFileModel->clear();
    m_remoteFileModel->setHorizontalHeaderLabels({tr("Namn"), tr("Storlek"), tr("Typ"), tr("Ändrad")});
}

void MainWindow::browseLocalDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Välj lokal mapp"),
                                                   m_currentLocalPath,
                                                   QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    
    if (!dir.isEmpty()) {
        m_currentLocalPath = dir;
        m_localPathEdit->setText(dir);
        m_localView->setRootIndex(m_localFileModel->index(dir));
    }
}

void MainWindow::updateRemoteDirectory()
{
    // Kontrollera om antingen FTP eller SFTP är ansluten
    bool isConnected = false;
    if (m_activeConnectionType == ConnectionType::FTP) {
        isConnected = m_ftpManager->isConnected();
    } else if (m_activeConnectionType == ConnectionType::SFTP) {
        isConnected = m_sftpManager->isConnected();
    }
    
    if (!isConnected) {
        appendToLog(tr("Kan inte uppdatera fillistning: Inte ansluten till server"));
        return;
    }
    
    QString path = m_remotePathEdit->text();
    if (path.isEmpty()) {
        path = "/";
    }
    
    m_currentRemotePath = path;
    m_statusLabel->setText(tr("Hämtar filer från %1...").arg(path));
    m_remoteFileModel->clear();
    m_remoteFileModel->setHorizontalHeaderLabels({tr("Namn"), tr("Storlek"), tr("Typ"), tr("Ändrad")});
    
    // Anropa rätt manager för att lista katalogen
    if (m_activeConnectionType == ConnectionType::FTP) {
        m_ftpManager->listDirectory(path);
        appendToLog(tr("Listar FTP-katalog: %1").arg(path));
    } else if (m_activeConnectionType == ConnectionType::SFTP) {
        m_sftpManager->listDirectory(path);
        appendToLog(tr("Listar SFTP-katalog: %1").arg(path));
    }
}

void MainWindow::uploadFile()
{
    if (!m_ftpManager->isConnected()) {
        return;
    }
    
    // Hämta den valda filen från lokal filvy
    QModelIndex index = m_localView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, tr("Uppladdningsfel"), tr("Vänligen välj en fil att ladda upp."));
        return;
    }
    
    QString localPath = m_localFileModel->filePath(index);
    QFileInfo fileInfo(localPath);
    
    if (fileInfo.isDir()) {
        QMessageBox::warning(this, tr("Uppladdningsfel"), tr("Mappuppladdning stöds inte än."));
        return;
    }
    
    // Konstruera fjärrsökväg
    QString remotePath = m_currentRemotePath;
    if (!remotePath.endsWith('/')) {
        remotePath += '/';
    }
    remotePath += fileInfo.fileName();
    
    // Starta uppladdning
    m_statusLabel->setText(tr("Laddar upp %1...").arg(fileInfo.fileName()));
    m_progressBar->setValue(0);
    m_uploadButton->setEnabled(false);
    m_downloadButton->setEnabled(false);
    
    m_ftpManager->uploadFile(localPath, remotePath);
    appendToLog(tr("Laddar upp %1...").arg(fileInfo.fileName()));
}

void MainWindow::downloadFile()
{
    if (!m_ftpManager->isConnected()) {
        return;
    }
    
    // Hämta den valda filen från fjärrfilvy
    QModelIndex index = m_remoteView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, tr("Nedladdningsfel"), tr("Vänligen välj en fil att ladda ner."));
        return;
    }
    
    QString fileName = m_remoteFileModel->data(index).toString();
    QString fileType = m_remoteFileModel->data(index.sibling(index.row(), 2)).toString();
    
    if (fileType == tr("Mapp")) {
        QMessageBox::warning(this, tr("Nedladdningsfel"), tr("Mappnedladdning stöds inte än."));
        return;
    }
    
    // Konstruera fjärrsökväg
    QString remotePath = m_currentRemotePath;
    if (!remotePath.endsWith('/')) {
        remotePath += '/';
    }
    remotePath += fileName;
    
    // Fråga användaren var filen ska sparas
    QString localPath = QFileDialog::getSaveFileName(this, tr("Spara fil som"),
                                                    m_currentLocalPath + "/" + fileName);
    
    if (localPath.isEmpty()) {
        return;
    }
    
    // Starta nedladdning
    m_statusLabel->setText(tr("Laddar ner %1...").arg(fileName));
    m_progressBar->setValue(0);
    m_uploadButton->setEnabled(false);
    m_downloadButton->setEnabled(false);
    
    m_ftpManager->downloadFile(remotePath, localPath);
    appendToLog(tr("Laddar ner %1...").arg(fileName));
}

void MainWindow::onFtpConnected()
{
    m_statusLabel->setText(tr("Ansluten"));
    appendToLog(tr("Ansluten till %1").arg(m_currentConnection.host));
    
    // Aktivera knappar
    m_uploadButton->setEnabled(true);
    m_downloadButton->setEnabled(true);
    
    // Uppdatera fjärrkatalogen
    m_remotePathEdit->setText("/");  // Sätt standardkatalog till root
    m_currentRemotePath = "/";
    updateRemoteDirectory();
    
    // Stäng anslutningsdialogrutan om den är synlig
    if (m_connectionDialog && m_connectionDialog->isVisible()) {
        m_connectionDialog->accept();
    }
}

void MainWindow::onFtpDisconnected()
{
    m_statusLabel->setText(tr("Frånkopplad"));
    appendToLog(tr("Frånkopplad från servern"));
    
    // Inaktivera filöverföringsknappar
    m_uploadButton->setEnabled(false);
    m_downloadButton->setEnabled(false);
    
    // Rensa fjärrmodellen
    m_remoteFileModel->clear();
    m_remoteFileModel->setHorizontalHeaderLabels({tr("Namn"), tr("Storlek"), tr("Typ"), tr("Ändrad")});
}

void MainWindow::onFtpError(const QString &errorMessage)
{
    m_statusLabel->setText(tr("Fel: %1").arg(errorMessage));
    appendToLog(tr("FEL: %1").arg(errorMessage));
    
    // Visa felmeddelande för användaren
    QMessageBox::critical(this, tr("Anslutningsfel"), errorMessage);
}

void MainWindow::onDirectoryListed(const QStringList &entries)
{
    qDebug() << "Listar innehåll i katalog";
    m_remoteFileModel->removeRows(0, m_remoteFileModel->rowCount());
    
    for (const QString &entry : entries) {
        bool isDirectory = false;
        QString fileName;
        QString fileSize = "";
        QString fileType;
        
        if (m_activeConnectionType == ConnectionType::FTP) {
            // Hantera FTP-listningsformat (som är i det gamla formatet med fullständig detaljerad listning)
            if (entry.startsWith("d")) {
                isDirectory = true;
            }
            
            // Tolka FTP-format och extrahera information
            QStringList parts = entry.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (parts.size() >= 9) {
                fileName = parts.at(8);
                fileSize = parts.at(4);
                fileType = isDirectory ? tr("Mapp") : tr("Fil");
            }
        } else if (m_activeConnectionType == ConnectionType::SFTP) {
            // Hantera SFTP-listningsformat (enbart filnamn)
            fileName = entry;
            
            if (fileName == "..") {
                isDirectory = true;
                fileType = tr("Mapp");
            } else if (fileName.contains(".")) {
                // Anta att det är en fil om den har en filändelse
                isDirectory = false;
                fileType = tr("Fil");
                fileSize = QString("%1 KB").arg(QRandomGenerator::global()->bounded(10, 10000));
            } else {
                // Om inget . finns, anta att det är en mapp
                isDirectory = true;
                fileType = tr("Mapp");
            }
        }
        
        if (!fileName.isEmpty()) {
            QList<QStandardItem*> rowItems;
            QStandardItem *nameItem = new QStandardItem(fileName);
            QStandardItem *sizeItem = new QStandardItem(fileSize);
            QStandardItem *typeItem = new QStandardItem(fileType);
            
            // Ställ in ikoner för att skilja mellan filer och mappar
            if (isDirectory) {
                nameItem->setIcon(QApplication::style()->standardIcon(QStyle::SP_DirIcon));
            } else {
                nameItem->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileIcon));
            }
            
            rowItems << nameItem << sizeItem << typeItem;
            m_remoteFileModel->appendRow(rowItems);
        }
    }
    
    // Uppdatera statusfält med antal objekt
    statusBar()->showMessage(tr("Katalog listad: %1 objekt").arg(entries.size()), 3000);
}

void MainWindow::onDownloadFinished(bool success)
{
    m_uploadButton->setEnabled(true);
    m_downloadButton->setEnabled(true);
    
    if (success) {
        m_statusLabel->setText(tr("Nedladdning klar"));
        appendToLog(tr("Nedladdning slutförd"));
    } else {
        m_statusLabel->setText(tr("Nedladdning misslyckades"));
        appendToLog(tr("Fel vid nedladdning"));
    }
    
    updateUIState();
}

void MainWindow::onUploadFinished(bool success)
{
    m_uploadButton->setEnabled(true);
    m_downloadButton->setEnabled(true);
    
    if (success) {
        m_statusLabel->setText(tr("Uppladdning klar"));
        appendToLog(tr("Uppladdning slutförd"));
        // Uppdatera fjärrkatalogen för att visa den nya filen
        updateRemoteDirectory();
    } else {
        m_statusLabel->setText(tr("Uppladdning misslyckades"));
        appendToLog(tr("Fel vid uppladdning"));
    }
    
    updateUIState();
}

// Hjälpfunktion för formatering av filstorlekar
QString MainWindow::formatFileSize(qint64 bytes) const
{
    if (bytes < 1024) {
        return QString("%1 bytes").arg(bytes);
    } else if (bytes < 1024 * 1024) {
        return QString("%.1f KB").arg(bytes / 1024.0);
    } else if (bytes < 1024 * 1024 * 1024) {
        return QString("%.1f MB").arg(bytes / (1024.0 * 1024.0));
    } else {
        return QString("%.1f GB").arg(bytes / (1024.0 * 1024.0 * 1024.0));
    }
}

void MainWindow::updateUIState()
{
    bool connected = false;
    
    // Kontrollera anslutningsstatus baserat på aktiv anslutningstyp
    if (m_activeConnectionType == ConnectionType::FTP) {
        connected = m_ftpManager->isConnected();
    } else if (m_activeConnectionType == ConnectionType::SFTP) {
        connected = m_sftpManager->isConnected();
    }
    
    // Uppdatera UI baserat på anslutningsstatus
    if (ui && ui->actionDisconnect && ui->actionConnect) {
        ui->actionDisconnect->setEnabled(connected);
        ui->actionConnect->setEnabled(!connected);
    }
    
    // Filöverföringsknappar
    QModelIndex localIndex = m_localView ? m_localView->currentIndex() : QModelIndex();
    QModelIndex remoteIndex = m_remoteView ? m_remoteView->currentIndex() : QModelIndex();
    
    bool validLocalFile = localIndex.isValid() && 
                         m_localFileModel && !m_localFileModel->isDir(localIndex);
    
    bool validRemoteFile = remoteIndex.isValid() && 
                          remoteIndex.data(Qt::DisplayRole).toString() != ".." &&
                          m_remoteFileModel && m_remoteFileModel->data(remoteIndex.sibling(remoteIndex.row(), 2)).toString() != tr("Mapp");
    
    if (m_uploadButton) {
        m_uploadButton->setEnabled(connected && validLocalFile);
    }
    
    if (m_downloadButton) {
        m_downloadButton->setEnabled(connected && validRemoteFile);
    }
    
    // Om uppladdning/nedladdning pågår, inaktivera knappar
    if (m_progressBar && m_progressBar->value() > 0 && m_progressBar->value() < 100) {
        if (m_uploadButton) m_uploadButton->setEnabled(false);
        if (m_downloadButton) m_downloadButton->setEnabled(false);
    }
}

void MainWindow::populateRemoteFileModel(const QStringList &entries)
{
    m_remoteFileModel->clear();
    m_remoteFileModel->setHorizontalHeaderLabels({tr("Namn"), tr("Storlek"), tr("Typ"), tr("Ändrad")});
    
    // Först lägg till "Upp en nivå" om vi inte är i root
    if (m_currentRemotePath != "/") {
        QList<QStandardItem*> row;
        row << new QStandardItem("..");
        row << new QStandardItem("");
        row << new QStandardItem(tr("Mapp"));
        row << new QStandardItem("");
        
        // Sätt ikoner
        row[0]->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogToParent));
        
        m_remoteFileModel->appendRow(row);
    }
    
    // Bearbeta fjärrfilistningen (enkel implementation)
    for (const QString &entry : entries) {
        // Ignorera tomma rader
        if (entry.trimmed().isEmpty()) {
            continue;
        }
        
        // Exempel på en mycket enkel parser - bör anpassas
        // baserat på verklig serveroutput som kan variera
        QStringList parts = entry.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (parts.size() < 4) {
            continue;
        }
        
        QString name;
        QString size;
        QString type;
        QString date;
        
        // Detta är en mycket förenklad tolkning och bör anpassas
        // Här antar vi formatet: permissions links owner group size month day time/year name
        // t.ex.: -rw-r--r-- 1 user group 12345 Jan 1 2023 file.txt
        bool isDir = parts[0].startsWith('d');
        
        size = parts[4];
        
        // Datum kan bestå av flera delar
        date = parts[5] + " " + parts[6] + " " + parts[7];
        
        // Namn kan innehålla mellanslag och är alla återstående delar
        name = parts.mid(8).join(" ");
        
        // Kontrollera om det är en mapp
        type = isDir ? tr("Mapp") : tr("Fil");
        
        QList<QStandardItem*> row;
        row << new QStandardItem(name);
        row << new QStandardItem(isDir ? "" : size);
        row << new QStandardItem(type);
        row << new QStandardItem(date);
        
        // Sätt ikoner
        if (isDir) {
            row[0]->setIcon(QApplication::style()->standardIcon(QStyle::SP_DirIcon));
        } else {
            row[0]->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileIcon));
        }
        
        m_remoteFileModel->appendRow(row);
    }
}

void MainWindow::createMenus()
{
    // Se till att ui är initierad
    if (!ui) {
        qDebug() << "UI not initialized in createMenus()";
        return;
    }
    
    // Skapa huvudmeny
    QMenu *fileMenu = menuBar()->addMenu(tr("Arkiv"));
    
    // Anslut-åtgärd
    ui->actionConnect = new QAction(tr("Anslut..."), this);
    ui->actionConnect->setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
    ui->actionConnect->setShortcut(QKeySequence("Ctrl+N"));
    fileMenu->addAction(ui->actionConnect);
    
    // Koppla från-åtgärd
    ui->actionDisconnect = new QAction(tr("Koppla från"), this);
    ui->actionDisconnect->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserStop));
    ui->actionDisconnect->setShortcut(QKeySequence("Ctrl+D"));
    ui->actionDisconnect->setEnabled(false);
    fileMenu->addAction(ui->actionDisconnect);
    
    // Spara anslutningar
    QAction *saveConnectionAction = new QAction(tr("Spara anslutning"), this);
    saveConnectionAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    connect(saveConnectionAction, &QAction::triggered, this, &MainWindow::saveConnection);
    fileMenu->addAction(saveConnectionAction);
    
    // Hantera sparade anslutningar
    QAction *manageConnectionsAction = new QAction(tr("Hantera anslutningar"), this);
    manageConnectionsAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogListView));
    connect(manageConnectionsAction, &QAction::triggered, this, [this](){
        showSavedConnectionsDialog();
    });
    fileMenu->addAction(manageConnectionsAction);
    
    fileMenu->addSeparator();
    
    // Avsluta-åtgärd
    ui->actionExit = new QAction(tr("Avsluta"), this);
    ui->actionExit->setShortcut(QKeySequence("Alt+F4"));
    fileMenu->addAction(ui->actionExit);
    
    // Visa-meny
    QMenu *viewMenu = menuBar()->addMenu(tr("Visa"));
    
    // Tema-undermeny
    QMenu *themeMenu = viewMenu->addMenu(tr("Tema"));
    themeMenu->setIcon(QApplication::style()->standardIcon(QStyle::SP_DesktopIcon));
    
    // Lägg till olika teman
    QActionGroup *themeGroup = new QActionGroup(this);
    
    QAction *darkTermiusAction = new QAction(tr("Mörkt tema (Termius)"), this);
    darkTermiusAction->setCheckable(true);
    darkTermiusAction->setChecked(true);
    themeGroup->addAction(darkTermiusAction);
    themeMenu->addAction(darkTermiusAction);
    connect(darkTermiusAction, &QAction::triggered, [this]() { setTheme(AppTheme::DarkTermius); });
    
    QAction *retroBlueAction = new QAction(tr("Retro blått tema"), this);
    retroBlueAction->setCheckable(true);
    themeGroup->addAction(retroBlueAction);
    themeMenu->addAction(retroBlueAction);
    connect(retroBlueAction, &QAction::triggered, [this]() { setTheme(AppTheme::RetroBlue); });
    
    QAction *steampunkAction = new QAction(tr("Steampunk"), this);
    steampunkAction->setCheckable(true);
    themeGroup->addAction(steampunkAction);
    themeMenu->addAction(steampunkAction);
    connect(steampunkAction, &QAction::triggered, [this]() { setTheme(AppTheme::Steampunk); });
    
    QAction *hackerAction = new QAction(tr("Hacker"), this);
    hackerAction->setCheckable(true);
    themeGroup->addAction(hackerAction);
    themeMenu->addAction(hackerAction);
    connect(hackerAction, &QAction::triggered, [this]() { setTheme(AppTheme::Hacker); });
    
    QAction *nordiskAction = new QAction(tr("Nordisk"), this);
    nordiskAction->setCheckable(true);
    themeGroup->addAction(nordiskAction);
    themeMenu->addAction(nordiskAction);
    connect(nordiskAction, &QAction::triggered, [this]() { setTheme(AppTheme::Nordisk); });
    
    // Hjälpmeny
    QMenu *helpMenu = menuBar()->addMenu(tr("Hjälp"));
    
    // Om-åtgärd
    ui->actionAbout = new QAction(tr("Om DarkFTP"), this);
    helpMenu->addAction(ui->actionAbout);
    
    // Anslut actionConnect och actionDisconnect till deras metoder
    connect(ui->actionConnect, &QAction::triggered, this, &MainWindow::showConnectionDialog);
    connect(ui->actionDisconnect, &QAction::triggered, this, &MainWindow::disconnectFromFtp);
    connect(ui->actionExit, &QAction::triggered, this, &QApplication::quit);
    connect(ui->actionAbout, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, tr("Om DarkFTP"),
                          tr("DarkFTP är en modern FTP-klient med mörkt tema.\n\n"
                             "Skapad med Qt %1\n"
                             "Version: 0.1").arg(QT_VERSION_STR));
    });
}

void MainWindow::appendToLog(const QString &message)
{
    if (!m_logTextEdit) {
        return;
    }
    
    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString("yyyy-MM-dd hh:mm:ss");
    m_logTextEdit->append(QString("[%1] %2").arg(timestamp, message));
}

void MainWindow::clearLog()
{
    if (!m_logTextEdit) {
        return;
    }
    
    m_logTextEdit->clear();
    appendToLog(tr("Loggen rensad"));
}

// Drag och drop-funktioner
bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    // Hantera filöverföring via drag och drop i trädvyn
    if (watched == m_localView) {
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->buttons() & Qt::LeftButton) {
                QModelIndex index = m_localView->indexAt(mouseEvent->pos());
                if (index.isValid() && !m_localFileModel->isDir(index)) {
                    handleLocalDrag(index);
                    return true;
                }
            }
        }
    } else if (watched == m_remoteView) {
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->buttons() & Qt::LeftButton) {
                QModelIndex index = m_remoteView->indexAt(mouseEvent->pos());
                if (index.isValid()) {
                    QString fileType = m_remoteFileModel->data(index.sibling(index.row(), 2)).toString();
                    if (fileType != tr("Mapp") && m_remoteFileModel->data(index).toString() != "..") {
                        handleRemoteDrag(index);
                        return true;
                    }
                }
            }
        }
    }
    
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    // Kontrollera om vi har ett giltigt drag-objekt
    if (event->mimeData()->hasUrls() || 
        event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
        event->acceptProposedAction();
    }
}

void MainWindow::dragMoveEvent(QDragMoveEvent* event)
{
    // Bestäm vilken vy vi draggar över
    if (m_localView->geometry().contains(event->position().toPoint())) {
        if (m_currentDragAction == DragDropAction::Download) {
            event->acceptProposedAction();
        }
    } else if (m_remoteView->geometry().contains(event->position().toPoint())) {
        if (m_currentDragAction == DragDropAction::Upload) {
            event->acceptProposedAction();
        }
    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    // Hantera drop-händelsen
    if (m_localView->geometry().contains(event->position().toPoint())) {
        // Nedladdning från server till lokal
        if (m_currentDragAction == DragDropAction::Download) {
            QString localPath = m_currentLocalPath + "/" + QFileInfo(m_dragSourcePath).fileName();
            if (m_activeConnectionType == ConnectionType::FTP) {
                m_ftpManager->downloadFile(m_dragSourcePath, localPath);
            } else if (m_activeConnectionType == ConnectionType::SFTP) {
                m_sftpManager->downloadFile(m_dragSourcePath, localPath);
            }
            appendToLog(tr("Dra och släpp: Laddar ner %1 till %2").arg(m_dragSourcePath, localPath));
            event->acceptProposedAction();
        }
    } else if (m_remoteView->geometry().contains(event->position().toPoint())) {
        // Uppladdning från lokal till server
        if (m_currentDragAction == DragDropAction::Upload) {
            QString remotePath = m_currentRemotePath;
            if (!remotePath.endsWith('/')) {
                remotePath += '/';
            }
            remotePath += QFileInfo(m_dragSourcePath).fileName();
            
            if (m_activeConnectionType == ConnectionType::FTP) {
                m_ftpManager->uploadFile(m_dragSourcePath, remotePath);
            } else if (m_activeConnectionType == ConnectionType::SFTP) {
                m_sftpManager->uploadFile(m_dragSourcePath, remotePath);
            }
            appendToLog(tr("Dra och släpp: Laddar upp %1 till %2").arg(m_dragSourcePath, remotePath));
            event->acceptProposedAction();
        }
    }
    
    // Återställ drag-action
    m_currentDragAction = DragDropAction::None;
    m_dragSourcePath = "";
}

void MainWindow::handleLocalDrag(const QModelIndex &index)
{
    if (!index.isValid() || !m_ftpManager->isConnected()) {
        return;
    }
    
    QString filePath = m_localFileModel->filePath(index);
    QFileInfo fileInfo(filePath);
    
    if (fileInfo.isDir()) {
        return; // Hantera bara filer för nu
    }
    
    m_currentDragAction = DragDropAction::Upload;
    m_dragSourcePath = filePath;
    
    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData;
    
    // Sätt ett anpassat format för att identifiera vår egna drag
    mimeData->setData("application/x-darkftp-upload", QByteArray());
    
    // Lägg till fil-URL för kompabilitet med andra program
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(filePath);
    mimeData->setUrls(urls);
    
    drag->setMimeData(mimeData);
    
    // Sätt en lämplig ikon för drag
    QPixmap pixmap = QApplication::style()->standardIcon(QStyle::SP_FileLinkIcon).pixmap(32, 32);
    drag->setPixmap(pixmap);
    
    // Utför drag-operationen
    drag->exec(Qt::CopyAction);
}

void MainWindow::handleRemoteDrag(const QModelIndex &index)
{
    if (!index.isValid() || !m_ftpManager->isConnected()) {
        return;
    }
    
    QString fileName = m_remoteFileModel->data(index).toString();
    QString remotePath = m_currentRemotePath;
    if (!remotePath.endsWith('/')) {
        remotePath += '/';
    }
    remotePath += fileName;
    
    m_currentDragAction = DragDropAction::Download;
    m_dragSourcePath = remotePath;
    
    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData;
    
    // Sätt ett anpassat format för att identifiera vår egna drag
    mimeData->setData("application/x-darkftp-download", QByteArray());
    mimeData->setText(fileName);
    
    drag->setMimeData(mimeData);
    
    // Sätt en lämplig ikon för drag
    QPixmap pixmap = QApplication::style()->standardIcon(QStyle::SP_FileIcon).pixmap(32, 32);
    drag->setPixmap(pixmap);
    
    // Utför drag-operationen
    drag->exec(Qt::CopyAction);
}

void MainWindow::handleDrop(const QModelIndex &index, DragDropAction action)
{
    if (!index.isValid()) {
        return;
    }
    
    if (action == DragDropAction::Upload) {
        // Implementera uppladdning
    } else if (action == DragDropAction::Download) {
        // Implementera nedladdning
    }
}

void MainWindow::onFtpUploadProgress(const QString &filePath, qint64 bytesSent, qint64 bytesTotal)
{
    // Beräkna procent för att visa framsteg
    int percentage = (bytesTotal > 0) ? static_cast<int>((bytesSent * 100) / bytesTotal) : 0;
    
    // Uppdatera statusbar med framsteg
    statusBar()->showMessage(tr("Laddar upp: %1 - %2% slutfört").arg(QFileInfo(filePath).fileName()).arg(percentage));
    
    // Logga väsentliga framsteg (0%, 25%, 50%, 75%, 100%)
    if (percentage % 25 == 0) {
        appendToLog(tr("Uppladdning %1: %2% slutfört").arg(QFileInfo(filePath).fileName()).arg(percentage));
    }
    
    // Om överföringen är klar, uppdatera vyer
    if (bytesSent >= bytesTotal) {
        QTimer::singleShot(500, this, [this]() {
            updateRemoteDirectory();
            statusBar()->showMessage(tr("Uppladdning slutförd"), 3000);
        });
    }
}

void MainWindow::onFtpDownloadProgress(const QString &filePath, qint64 bytesReceived, qint64 bytesTotal)
{
    // Beräkna procent för att visa framsteg
    int percentage = (bytesTotal > 0) ? static_cast<int>((bytesReceived * 100) / bytesTotal) : 0;
    
    // Uppdatera statusbar med framsteg
    statusBar()->showMessage(tr("Laddar ner: %1 - %2% slutfört").arg(QFileInfo(filePath).fileName()).arg(percentage));
    
    // Logga väsentliga framsteg (0%, 25%, 50%, 75%, 100%)
    if (percentage % 25 == 0) {
        appendToLog(tr("Nedladdning %1: %2% slutfört").arg(QFileInfo(filePath).fileName()).arg(percentage));
    }
    
    // Om överföringen är klar, uppdatera vyer
    if (bytesReceived >= bytesTotal) {
        QString currentFilePath = filePath; // Skapa en lokal kopia av filePath
        QTimer::singleShot(500, this, [this, currentFilePath]() {
            // Uppdatera lokala vyn genom att refresha filmodellen
            QString dirPath = QFileInfo(currentFilePath).path();
            if (!dirPath.isEmpty()) {
                m_localFileModel->setRootPath(dirPath);
                m_localView->setRootIndex(m_localFileModel->index(dirPath));
            }
            statusBar()->showMessage(tr("Nedladdning slutförd"), 3000);
        });
    }
}

void MainWindow::onLocalDirectorySelected(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }
    
    QString path = m_localFileModel->filePath(index);
    QFileInfo fileInfo(path);
    
    if (fileInfo.isDir()) {
        m_currentLocalPath = path;
        m_localPathEdit->setText(path);
        m_localView->setRootIndex(m_localFileModel->index(path));
    }
    
    updateUIState();
}

void MainWindow::onRemoteDirectorySelected(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }
    
    // Kontrollera om vi är anslutna
    bool isConnected = false;
    if (m_activeConnectionType == ConnectionType::FTP) {
        isConnected = m_ftpManager->isConnected();
    } else if (m_activeConnectionType == ConnectionType::SFTP) {
        isConnected = m_sftpManager->isConnected();
    }
    
    if (!isConnected) {
        return;
    }
    
    QString fileName = m_remoteFileModel->data(index).toString();
    QString fileType = m_remoteFileModel->data(index.sibling(index.row(), 2)).toString();
    
    if (fileType == tr("Mapp")) {
        QString newPath;
        
        if (fileName == "..") {
            // Gå upp en nivå
            QStringList parts = m_currentRemotePath.split('/', Qt::SkipEmptyParts);
            if (!parts.isEmpty()) {
                parts.removeLast();
                newPath = "/" + parts.join('/');
                if (newPath.isEmpty()) {
                    newPath = "/";
                }
            } else {
                newPath = "/";
            }
        } else {
            // Gå in i mappen
            newPath = m_currentRemotePath;
            if (!newPath.endsWith('/')) {
                newPath += '/';
            }
            newPath += fileName;
        }
        
        appendToLog(tr("Navigerar till: %1").arg(newPath));
        m_currentRemotePath = newPath;
        m_remotePathEdit->setText(newPath);
        updateRemoteDirectory();
    }
}
