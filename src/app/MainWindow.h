#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringListModel>
#include <QVector>
#include "connection.h" // Assuming connection.h is in the same directory or include path is set

// Forward declarations
class QLineEdit;
class QPushButton;
class QListView;
class QTabWidget;
class FtpManager; // Assuming FtpManager class exists
class SftpManager; // Assuming SftpManager class exists
class QInputDialog; // For prompting passwords
class QCryptographicHash; // For hashing
class QByteArray; // For hashing

struct TabInfo {
    Connection connectionInfo;
    // Other tab-specific info
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    // Modified signature
    void connectFromQmlEx(const QString &host, const QString &port, const QString &username, const QString &password, const QString &protocol, const QString &initialLocalPath, const QString &initialRemotePath, bool saveConnectionChecked);
    void saveConnection(); // To be reviewed and refined
    void connectToServer(const Connection &connectionDetails); // To be modified

private slots:
    // Other slots as needed

private:
    void loadSettings();
    void saveSettings(); // Might be relevant for how saved connections are stored/loaded

    Connection m_currentConnection;
    QVector<Connection> m_savedConnections;
    QVector<TabInfo> m_tabs; // Assuming m_tabs[m_currentTabIndex].connectionInfo is how current tab's connection is accessed
    int m_currentTabIndex = 0; // Assuming this exists

    // UI elements, if needed for context, though not directly modified by this subtask
    // QLineEdit *m_hostEdit;
    // ... other UI elements

    FtpManager *m_ftpManager;   // Assuming you have an FtpManager
    SftpManager *m_sftpManager; // Assuming you have an SftpManager

    // Helper for hashing
    QString hashPassword(const QString& plainPassword);
    bool isPasswordHashed(const QString& password); // Helper to check if password looks hashed
};

#endif // MAINWINDOW_H
