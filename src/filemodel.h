#ifndef FILEMODEL_H
#define FILEMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QString>
#include <QVariant>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QUrl>

// Struktur för att hålla filinformation
struct FileInfo {
    QString fileName;
    QString filePath;
    qint64 fileSize;
    QDateTime fileDate;
    bool isDirectory;
    // Lägg till fler attribut senare (t.ex. permissions)
};

class FileModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString currentPath READ currentPath WRITE setCurrentPath NOTIFY currentPathChanged)
    Q_PROPERTY(bool isRemote READ isRemote CONSTANT) // För att skilja på lokal/remote

public:
    // Roller för att exponera data till QML
    enum FileRoles {
        FileNameRole = Qt::UserRole + 1,
        FilePathRole,
        FileSizeRole,
        FileDateRole,
        IsDirectoryRole
        // Lägg till fler roller senare
    };

    explicit FileModel(bool remote = false, QObject *parent = nullptr);

    // === QAbstractListModel Overrides ===
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // === Offentliga metoder ===
    Q_INVOKABLE void navigate(const QString &path); // Navigera till en sökväg
    Q_INVOKABLE void refresh(); // Uppdatera aktuell kataloglistning
    Q_INVOKABLE void goUp();    // Gå upp en nivå
    Q_INVOKABLE bool createDirectory(const QString &name); // Ny metod
    Q_INVOKABLE bool deletePath(const QString &path);     // Ny metod
    Q_INVOKABLE bool renamePath(const QString &oldPath, const QString &newName); // Ny metod
    Q_INVOKABLE QVariantMap get(int index) const; // Ny metod

    QString currentPath() const;
    bool isRemote() const;

public slots:
    void setCurrentPath(const QString &path);

signals:
    void currentPathChanged(const QString &path);
    void error(const QString &message); // Signal för felmeddelanden

private:
    void listDirectory(const QString &path); // Intern metod för att lista katalog

    QList<FileInfo> m_files;
    QString m_currentPath;
    bool m_isRemote;
    QHash<int, QByteArray> m_roleNames;
};

#endif // FILEMODEL_H 