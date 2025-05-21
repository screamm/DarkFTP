#ifndef CONNECTION_H
#define CONNECTION_H

#include <QString>

struct Connection {
    enum Protocol { FTP, SFTP };
    enum SftpAuthMethod { PasswordAuth, KeyAuth, BothAuth };

    QString host;
    int port = 0;
    QString username;
    QString password;
    Protocol protocol = FTP;
    QString initialLocalPath;
    QString initialRemotePath;
    bool savePassword = false; // New field
    QString privateKeyPath;
    QString keyPassphrase;
    SftpAuthMethod sftpAuthMethod = PasswordAuth; // Assuming this exists for SFTP

    // Helper to check if password is required for the current configuration
    bool isPasswordAuthRequired() const {
        if (protocol == FTP) return true;
        if (protocol == SFTP && (sftpAuthMethod == PasswordAuth || sftpAuthMethod == BothAuth)) return true;
        return false;
    }

    // Helper to check if key passphrase is required
    bool isKeyAuthRequired() const {
        if (protocol == SFTP && (sftpAuthMethod == KeyAuth || sftpAuthMethod == BothAuth) && !privateKeyPath.isEmpty()) return true;
        return false;
    }
};

#endif // CONNECTION_H
