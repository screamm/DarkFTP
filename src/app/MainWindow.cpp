#include "MainWindow.h"
#include "connection.h" // Included via MainWindow.h, but good for clarity

#include <QDebug> // For logging
#include <QInputDialog>
#include <QCryptographicHash>
#include <QByteArray>
#include <QMessageBox> // For error messages or notifications

// Assuming FtpManager and SftpManager have connect methods like:
// void FtpManager::connectToHost(const QString &host, int port, const QString &username, const QString &password);
// void SftpManager::connectToHost(const QString &host, int port, const QString &username, const QString &password);
// void SftpManager::connectToHostWithKey(const QString &host, int port, const QString &username, const QString &privateKeyPath, const QString &keyPassphrase);
// These are placeholders; actual methods might differ.
// For simplicity, I'm assuming FtpManager and SftpManager are initialized in MainWindow's constructor.

// Define a prefix to identify hashed passwords
const QString HASH_PREFIX = "hashed_"; // Simple prefix for demonstration

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_ftpManager(nullptr), m_sftpManager(nullptr)
{
    // Initialize m_ftpManager and m_sftpManager, e.g.:
    // m_ftpManager = new FtpManager(this);
    // m_sftpManager = new SftpManager(this);

    // Load saved connections (potentially from QSettings)
    loadSettings();
}

MainWindow::~MainWindow()
{
    // Save settings on exit
    saveSettings(); // This would persist m_savedConnections

    // delete m_ftpManager;
    // delete m_sftpManager;
}

// Helper to hash a password
QString MainWindow::hashPassword(const QString& plainPassword) {
    if (plainPassword.isEmpty()) return QString();
    QByteArray data = plainPassword.toUtf8();
    QByteArray hashedData = QCryptographicHash::hash(data, QCryptographicHash::Sha256).toBase64();
    return HASH_PREFIX + QString(hashedData); // Add prefix to distinguish
}

// Helper to check if a password looks like it's hashed by our method
bool MainWindow::isPasswordHashed(const QString& password) {
    // A more robust check might involve trying to Base64-decode
    // or checking length and character set.
    // For this example, we'll rely on the prefix.
    return password.startsWith(HASH_PREFIX);
}

void MainWindow::connectFromQmlEx(const QString &host, const QString &portStr, const QString &username,
                                  const QString &password, const QString &protocolStr,
                                  const QString &initialLocalPath, const QString &initialRemotePath,
                                  bool saveConnectionChecked)
{
    qDebug() << "connectFromQmlEx called with saveConnectionChecked:" << saveConnectionChecked;

    Connection connection;
    connection.host = host;
    bool ok;
    connection.port = portStr.toInt(&ok);
    if (!ok || portStr.isEmpty()) { // Default port if empty or invalid
        connection.port = (protocolStr.toLower() == "sftp") ? 22 : 21;
    }
    connection.username = username;
    connection.password = password; // Plaintext from dialog for now
    connection.initialLocalPath = initialLocalPath;
    connection.initialRemotePath = initialRemotePath;
    connection.savePassword = saveConnectionChecked; // Set the flag

    if (protocolStr.toLower() == "sftp") {
        connection.protocol = Connection::SFTP;
        // For SFTP, we might need to determine auth method (password or key)
        // This example assumes password auth if password is provided,
        // or could be extended for key auth from QML.
        // For now, let's assume if password is provided, it's PasswordAuth.
        // If no password and a key path was provided from QML (not in this signature), it would be KeyAuth.
        // This part needs alignment with how QML provides key info.
        // Assuming QML sends password for password auth, and key details for key auth.
        // For simplicity, if password is not empty, consider it PasswordAuth.
        // A more complete solution would have QML send the intended auth method.
        if (!password.isEmpty()) {
            connection.sftpAuthMethod = Connection::PasswordAuth;
        } else {
            // If password is empty, and we are to use SFTP, we'd need key details.
            // This example doesn't receive key path/passphrase directly from QML in this function.
            // We might need to fetch them if saveConnectionChecked is true and a saved connection matches.
            // Or, QML dialog should provide these. For now, this is a simplification.
            // Let's assume if password is provided, it's password auth.
            // If password is not provided, and it's SFTP, it implies key auth or anonymous.
            // This example will primarily focus on password handling from the dialog.
        }
    } else {
        connection.protocol = Connection::FTP;
    }

    // The problem description implies connectToServer updates m_currentConnection
    connectToServer(connection);

    // If saveConnectionChecked is true, then save the connection details.
    // m_currentConnection should have been updated by connectToServer, including the savePassword flag.
    if (saveConnectionChecked) {
        // Ensure m_currentConnection reflects the 'connection' we just built, especially 'savePassword'
        // connectToServer already updates m_currentConnection.
        // We need to make sure the savePassword flag from the dialog is part of what might be saved.
        // The current structure is that connectToServer updates m_currentConnection,
        // so if 'connection' passed to it has savePassword=true, m_currentConnection will too.
        this->saveConnection();
    }
}

void MainWindow::saveConnection()
{
    qDebug() << "saveConnection called. m_currentConnection.savePassword is:" << m_currentConnection.savePassword;

    if (!m_currentConnection.savePassword) {
        qDebug() << "Password saving not requested. Aborting saveConnection.";
        return; // Only proceed if the flag is true
    }

    Connection connToSave = m_currentConnection; // Work with a copy

    // Hash password if it's plaintext and saving is enabled
    if (connToSave.isPasswordAuthRequired() && !connToSave.password.isEmpty() && !isPasswordHashed(connToSave.password)) {
        qDebug() << "Hashing password for connection:" << connToSave.host;
        connToSave.password = hashPassword(connToSave.password);
    }

    // Hash SSH key passphrase if present, plaintext, and saving is enabled
    if (connToSave.protocol == Connection::SFTP && connToSave.isKeyAuthRequired() && !connToSave.keyPassphrase.isEmpty() && !isPasswordHashed(connToSave.keyPassphrase)) {
        qDebug() << "Hashing key passphrase for connection:" << connToSave.host;
        connToSave.keyPassphrase = hashPassword(connToSave.keyPassphrase);
    }

    // Check if this connection already exists in m_savedConnections
    // This simple check uses host and username. A more robust check might include protocol and port.
    int existingIndex = -1;
    for (int i = 0; i < m_savedConnections.size(); ++i) {
        if (m_savedConnections[i].host == connToSave.host &&
            m_savedConnections[i].username == connToSave.username &&
            m_savedConnections[i].protocol == connToSave.protocol) {
            existingIndex = i;
            break;
        }
    }

    if (existingIndex != -1) {
        qDebug() << "Updating existing saved connection for host:" << connToSave.host;
        m_savedConnections[existingIndex] = connToSave;
    } else {
        qDebug() << "Adding new saved connection for host:" << connToSave.host;
        m_savedConnections.append(connToSave);
    }

    // After modifying m_savedConnections, settings should be persisted.
    // saveSettings(); // Calling saveSettings() here would write to QSettings immediately.
    // Depending on application design, saveSettings might be called on exit or explicitly by user.
    // For this refactor, we ensure m_savedConnections is correctly updated.
    // The problem implies saveSettings handles hashing for QSettings persistence,
    // so we ensure data *entering* m_savedConnections is also prepared.
}


void MainWindow::connectToServer(const Connection &connectionDetails)
{
    qDebug() << "connectToServer called for host:" << connectionDetails.host;
    Connection operationalConnection = connectionDetails; // Make a mutable copy

    // --- Password Handling (FTP & SFTP password/both auth) ---
    if (operationalConnection.isPasswordAuthRequired() &&
        !operationalConnection.password.isEmpty() &&
        isPasswordHashed(operationalConnection.password)) {
        qDebug() << "Password for" << operationalConnection.host << "is hashed. Prompting for plaintext.";
        bool ok;
        QString plainPassword = QInputDialog::getText(this, tr("Password Required"),
                                                      tr("Enter password for %1@%2").arg(operationalConnection.username, operationalConnection.host),
                                                      QLineEdit::Password, QString(), &ok);
        if (ok && !plainPassword.isEmpty()) {
            operationalConnection.password = plainPassword; // Use plaintext for this session
        } else if (!ok) { // User cancelled
            qDebug() << "User cancelled password input. Aborting connection.";
            QMessageBox::warning(this, "Connection Cancelled", "User cancelled password input.");
            return;
        }
        // If ok is true but plainPassword is empty, we proceed with an empty password.
    }

    // --- SFTP Key Passphrase Handling (SFTP key/both auth) ---
    if (operationalConnection.protocol == Connection::SFTP &&
        operationalConnection.isKeyAuthRequired() && // Checks if privateKeyPath is not empty
        !operationalConnection.keyPassphrase.isEmpty() &&
        isPasswordHashed(operationalConnection.keyPassphrase)) {
        qDebug() << "Key passphrase for" << operationalConnection.host << "is hashed. Prompting for plaintext.";
        bool ok;
        QString plainPassphrase = QInputDialog::getText(this, tr("SSH Key Passphrase Required"),
                                                       tr("Enter passphrase for key %1").arg(operationalConnection.privateKeyPath),
                                                       QLineEdit::Password, QString(), &ok);
        if (ok && !plainPassphrase.isEmpty()) {
            operationalConnection.keyPassphrase = plainPassphrase; // Use plaintext for this session
        } else if (!ok) { // User cancelled
            qDebug() << "User cancelled key passphrase input. Aborting connection.";
            QMessageBox::warning(this, "Connection Cancelled", "User cancelled key passphrase input.");
            return;
        }
        // If ok is true but plainPassphrase is empty, we proceed with an empty passphrase.
    }

    // Update m_currentConnection and tab info with the operational connection details
    // (which now contain plaintext credentials if they were prompted).
    m_currentConnection = operationalConnection;
    if (m_tabs.isEmpty()) { // Should not happen if a tab is always open or created before connect
        m_tabs.resize(1); // Ensure there's at least one tab
        m_currentTabIndex = 0;
         qWarning() << "connectToServer: m_tabs was empty, resized to 1. This might indicate an issue.";
    } else if (m_currentTabIndex < 0 || m_currentTabIndex >= m_tabs.size()) {
        qWarning() << "connectToServer: m_currentTabIndex is out of bounds. Resetting to 0.";
        m_currentTabIndex = 0; // Or handle error appropriately
    }
    m_tabs[m_currentTabIndex].connectionInfo = operationalConnection;


    qDebug() << "Proceeding to connect with" << (operationalConnection.protocol == Connection::SFTP ? "SFTP" : "FTP")
             << "for host" << operationalConnection.host;

    // Actual connection logic using m_ftpManager or m_sftpManager
    if (operationalConnection.protocol == Connection::FTP) {
        if (m_ftpManager) {
            // m_ftpManager->connectToHost(operationalConnection.host, operationalConnection.port,
            //                            operationalConnection.username, operationalConnection.password);
            qDebug() << "FTP connect call placeholder for" << operationalConnection.host;
        } else {
            qWarning() << "FTP Manager is null!";
        }
    } else if (operationalConnection.protocol == Connection::SFTP) {
        if (m_sftpManager) {
            if (operationalConnection.sftpAuthMethod == Connection::KeyAuth ||
                (operationalConnection.sftpAuthMethod == Connection::BothAuth && !operationalConnection.privateKeyPath.isEmpty())) {
                // m_sftpManager->connectToHostWithKey(operationalConnection.host, operationalConnection.port,
                //                                   operationalConnection.username, operationalConnection.privateKeyPath,
                //                                   operationalConnection.keyPassphrase);
                qDebug() << "SFTP connect (key auth) call placeholder for" << operationalConnection.host;
            } else { // PasswordAuth or BothAuth falling back to password
                // m_sftpManager->connectToHost(operationalConnection.host, operationalConnection.port,
                //                             operationalConnection.username, operationalConnection.password);
                qDebug() << "SFTP connect (password auth) call placeholder for" << operationalConnection.host;
            }
        } else {
            qWarning() << "SFTP Manager is null!";
        }
    } else {
        qWarning() << "Unknown protocol selected for host" << operationalConnection.host;
        QMessageBox::critical(this, "Connection Error", "Unknown protocol selected.");
        return;
    }
    // Connection status (success/failure) would typically be handled via signals/slots from managers.
}


// Placeholder for loadSettings
void MainWindow::loadSettings() {
    qDebug() << "loadSettings() called. (Placeholder)";
    // In a real app, this would load from QSettings, potentially decrypting/de-hashing if needed.
    // For this subtask, we assume m_savedConnections is populated correctly if it's loaded from elsewhere.
    // If passwords/passphrases are stored hashed in QSettings, then they'd be loaded as such.
    // The logic in connectToServer is designed to handle these pre-hashed credentials.
}

// Placeholder for saveSettings
void MainWindow::saveSettings() {
    qDebug() << "saveSettings() called. (Placeholder)";
    // In a real app, this would save m_savedConnections to QSettings.
    // The problem statement mentions: "The existing saveSettings hashes passphrases
    // from m_savedConnections before writing to QSettings".
    // This means if m_currentConnection (and thus entries in m_savedConnections)
    // has savePassword=true, its password/passphrase should already be hashed by saveConnection().
    // If savePassword=false, they'd be plaintext (or empty) and saveSettings might choose not to save them,
    // or save them as-is if that's the desired behavior for non-saved passwords.
    // This function would iterate m_savedConnections and write them.
    // If a connection in m_savedConnections has its password/passphrase already hashed
    // (because savePassword was true when it was last saved), saveSettings writes it as is.
    // If it's not hashed (savePassword was false), it might skip saving it or save it plaintext.
    // The critical part is that saveConnection ensures that if savePassword is true, the data
    // *in m_savedConnections* is already in the desired (hashed) format.
}

// Regarding Connection::isUseSSH:
// The provided connection.h does not have an 'isUseSSH' field.
// The logic should rely on 'connection.protocol == Connection::SFTP'.
// Any existing code using a standalone 'isUseSSH' boolean would need to be updated
// to check the protocol directly from the Connection object.
// This subtask focuses on credential handling, so direct modification of
// upload/download methods is out of scope unless 'isUseSSH' directly impacts
// how credentials are retrieved or used by those methods, which doesn't seem to be the case.
// The key is that m_currentConnection.protocol is correctly set and used.
// The connectToServer function correctly uses operationalConnection.protocol.
// If m_currentConnection is used by other functions like upload/download,
// they should also use m_currentConnection.protocol.
```
