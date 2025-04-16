#ifndef FILEMODEL_H
#define FILEMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QString>
#include <QVariant>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QCache>
#include <QVector>
#include <QMutex>
#include <QTimer>

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
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(bool hasMoreItems READ hasMoreItems NOTIFY loadingChanged)

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
    Q_INVOKABLE bool createDirectory(const QString &name);
    Q_INVOKABLE bool deletePath(const QString &path);
    Q_INVOKABLE bool renamePath(const QString &oldPath, const QString &newName);
    Q_INVOKABLE QVariantMap get(int index) const;

    // Egenskapsmetoder
    QString currentPath() const;
    void setCurrentPath(const QString &path);
    bool isRemote() const;
    bool isLoading() const;
    bool hasMoreItems() const;

    // Metoder som används av FileSystemTask
    void listDirectoryTask(const QString &path);
    void deletePathTask(const QString &path);
    void createDirectoryTask(const QString &basePath, const QString &name);
    void renamePathTask(const QString &oldPath, const QString &newPath);

signals:
    void currentPathChanged(const QString &path);
    void error(const QString &message);
    void loadingChanged();

private slots:
    // Callback-metoder för asynkrona operationer
    void createDirectoryResult(bool success, const QString &errorMsg);
    void deletePathResult(bool success, const QString &errorMsg);
    void renamePathResult(bool success, const QString &errorMsg);
    void addItem(const FileInfo &item, bool prepend = false);

private:
    // Interna hjälpmetoder
    bool isRoot(const QString &path) const;
    QString formatFileSize(qint64 size) const;
    void loadNextBatch(const QVector<FileInfo> &allFiles, int startIndex);

    // Medlemsvariabler
    QList<FileInfo> m_files;
    QString m_currentPath;
    bool m_isRemote;
    bool m_isLoading;
    bool m_hasMoreItems;
    QHash<int, QByteArray> m_roleNames;
    
    // Cache för kataloglistningar
    QCache<QString, QVector<FileInfo>> m_dirCache;
    
    // Mutex för trådsäkerhet
    QMutex m_mutex;
};

#endif // FILEMODEL_H 