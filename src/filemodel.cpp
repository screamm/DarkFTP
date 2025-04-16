#include "filemodel.h"
#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QDir>
#include <QMessageBox> // För framtida bekräftelsedialoger kanske
#include <QDateTime>
#include <QMutexLocker>
#include <QThreadPool>
#include <QRunnable>

// Konstanter för inkrementell laddning
const int BATCH_SIZE = 100;
const int MAX_CACHE_DIRS = 10;

// Hjälpklass för att utföra filsystemsoperationer i bakgrunden
class FileSystemTask : public QRunnable
{
public:
    enum TaskType {
        ListDirectory,
        DeleteFile,
        CreateDirectory,
        RenameFile
    };
    
    FileSystemTask(FileModel* model, TaskType type, const QString& arg1, const QString& arg2 = QString())
        : m_model(model), m_type(type), m_arg1(arg1), m_arg2(arg2)
    {
        setAutoDelete(true);
    }
    
    void run() override
    {
        switch (m_type) {
            case ListDirectory:
                m_model->listDirectoryTask(m_arg1);
                break;
            case DeleteFile:
                m_model->deletePathTask(m_arg1);
                break;
            case CreateDirectory:
                m_model->createDirectoryTask(m_arg1, m_arg2);
                break;
            case RenameFile:
                m_model->renamePathTask(m_arg1, m_arg2);
                break;
        }
    }
    
private:
    FileModel* m_model;
    TaskType m_type;
    QString m_arg1;
    QString m_arg2;
};

FileModel::FileModel(bool remote, QObject *parent)
    : QAbstractListModel(parent)
    , m_isRemote(remote)
    , m_isLoading(false)
    , m_hasMoreItems(false)
{
    // Definiera rollnamn för mappning till QML
    m_roleNames[FileNameRole] = "fileName";
    m_roleNames[FilePathRole] = "filePath";
    m_roleNames[FileSizeRole] = "fileSize";
    m_roleNames[FileDateRole] = "fileDate";
    m_roleNames[IsDirectoryRole] = "isDirectory";
    
    // Initiera cache-strukturer
    m_dirCache.setMaxCost(MAX_CACHE_DIRS);
}

int FileModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_files.size();
}

QVariant FileModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_files.size())
        return QVariant();
    
    const FileInfo &fileInfo = m_files.at(index.row());
    
    switch (role) {
        case FileNameRole:
            return fileInfo.fileName;
        case FilePathRole:
            return fileInfo.filePath;
        case FileSizeRole:
            return fileInfo.isDirectory ? "" : formatFileSize(fileInfo.fileSize);
        case FileDateRole:
            return fileInfo.fileDate.toString("yyyy-MM-dd hh:mm");
        case IsDirectoryRole:
            return fileInfo.isDirectory;
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> FileModel::roleNames() const
{
    return m_roleNames;
}

void FileModel::navigate(const QString &path)
{
    QString cleanedPath = path;
    
    // Säkerställ konsekvent hantering av snedstreck
    cleanedPath.replace('\\', '/');
    
    // Om vi redan har denna katalog i cachen, använd den direkt
    if (m_dirCache.contains(cleanedPath)) {
        QMutexLocker locker(&m_mutex);
        beginResetModel();
        m_files.clear();
        QVector<FileInfo>* cachedFiles = m_dirCache.object(cleanedPath);
        if (cachedFiles) {
            for (const FileInfo &info : *cachedFiles) {
                m_files.append(info);
            }
        }
        m_currentPath = cleanedPath;
        endResetModel();
        emit currentPathChanged(m_currentPath);
        return;
    }
    
    // Annars, ladda asynkront
    m_isLoading = true;
    emit loadingChanged();
    
    QFileInfo pathInfo(cleanedPath);
    if (!pathInfo.exists() || !pathInfo.isDir()) {
        emit error(tr("Katalogen finns inte: %1").arg(cleanedPath));
        m_isLoading = false;
        emit loadingChanged();
        return;
    }
    
    // Starta asynkron laddning
    m_currentPath = cleanedPath;
    emit currentPathChanged(m_currentPath);
    
    // Rensa modellen och visa "laddar"-indikation
    beginResetModel();
    m_files.clear();
    endResetModel();
    
    // Kör listning av katalog i en bakgrundstråd
    FileSystemTask* task = new FileSystemTask(this, 
                                            FileSystemTask::ListDirectory, 
                                            cleanedPath);
    QThreadPool::globalInstance()->start(task);
}

void FileModel::listDirectoryTask(const QString &path)
{
    QMutexLocker locker(&m_mutex);
    
    // Läs kataloginnehåll
    QDir dir(path);
    dir.setFilter(QDir::AllEntries | QDir::NoDot);
    QFileInfoList entries = dir.entryInfoList();
    
    QVector<FileInfo> files;
    
    // Sortera först kataloger, sen filer
    for (const QFileInfo &entry : entries) {
        if (entry.isDir()) {
            FileInfo info;
            info.fileName = entry.fileName();
            info.filePath = entry.filePath();
            info.fileSize = 0; // Kataloger har ingen storlek
            info.fileDate = entry.lastModified();
            info.isDirectory = true;
            files.append(info);
        }
    }
    
    for (const QFileInfo &entry : entries) {
        if (!entry.isDir()) {
            FileInfo info;
            info.fileName = entry.fileName();
            info.filePath = entry.filePath();
            info.fileSize = entry.size();
            info.fileDate = entry.lastModified();
            info.isDirectory = false;
            files.append(info);
        }
    }
    
    // Lägg till i cache
    m_dirCache.insert(path, new QVector<FileInfo>(files), files.size());
    
    // Lägg till först den speciella ".." uppkatalogen om vi inte är i root
    QDir currentDir(m_currentPath);
    if (currentDir.exists() && !isRoot(m_currentPath)) {
        FileInfo parentInfo;
        parentInfo.fileName = "..";
        parentInfo.filePath = currentDir.absolutePath() + "/..";
        parentInfo.fileSize = 0;
        parentInfo.fileDate = QDateTime::currentDateTime();
        parentInfo.isDirectory = true;
        
        QMetaObject::invokeMethod(this, "addItem", Qt::QueuedConnection,
                                 Q_ARG(FileInfo, parentInfo), Q_ARG(bool, true));
    }
    
    // Ladda första batchen
    loadNextBatch(files, 0);
}

void FileModel::loadNextBatch(const QVector<FileInfo> &allFiles, int startIndex)
{
    if (startIndex >= allFiles.size()) {
        // Alla filer är inlästa
        m_hasMoreItems = false;
        m_isLoading = false;
        emit loadingChanged();
        return;
    }
    
    // Bestäm slutindex för denna batch
    int endIndex = qMin(startIndex + BATCH_SIZE, allFiles.size());
    
    // Lägg till filerna inkrementellt
    for (int i = startIndex; i < endIndex; ++i) {
        QMetaObject::invokeMethod(this, "addItem", Qt::QueuedConnection,
                                 Q_ARG(FileInfo, allFiles[i]));
    }
    
    // Om det finns fler filer, ladda nästa batch efter kort fördröjning
    if (endIndex < allFiles.size()) {
        m_hasMoreItems = true;
        QTimer::singleShot(10, this, [this, allFiles, endIndex]() {
            loadNextBatch(allFiles, endIndex);
        });
    } else {
        // Alla filer är nu inlästa
        m_hasMoreItems = false;
        m_isLoading = false;
        emit loadingChanged();
    }
}

void FileModel::addItem(const FileInfo &item, bool prepend)
{
    // Lägg till en ny post i modellen
    if (prepend) {
        beginInsertRows(QModelIndex(), 0, 0);
        m_files.prepend(item);
        endInsertRows();
    } else {
        beginInsertRows(QModelIndex(), m_files.size(), m_files.size());
        m_files.append(item);
        endInsertRows();
    }
}

void FileModel::refresh()
{
    if (m_currentPath.isEmpty())
        return;
    
    // Rensa cachen för denna katalog så att den laddas om
    m_dirCache.remove(m_currentPath);
    
    // Navigera till samma katalog igen för att ladda om
    navigate(m_currentPath);
}

void FileModel::goUp()
{
    if (m_currentPath.isEmpty())
        return;
    
    QDir currentDir(m_currentPath);
    if (currentDir.cdUp()) {
        navigate(currentDir.absolutePath());
    }
}

bool FileModel::isRoot(const QString &path) const
{
#ifdef Q_OS_WIN
    // På Windows, kontrollera om detta är en rotbokstav som C:/ eller D:/
    return path.length() <= 3 && path.contains(":/");
#else
    // På Unix-system, kontrollera om detta är /-katalogen
    return path == "/";
#endif
}

bool FileModel::createDirectory(const QString &name)
{
    if (m_currentPath.isEmpty())
        return false;
    
    FileSystemTask* task = new FileSystemTask(this, 
                                            FileSystemTask::CreateDirectory, 
                                            m_currentPath, name);
    QThreadPool::globalInstance()->start(task);
    
    return true;
}

void FileModel::createDirectoryTask(const QString &basePath, const QString &name)
{
    bool success = false;
    QString errorMsg;
    
    QDir dir(basePath);
    success = dir.mkdir(name);
    
    if (!success) {
        errorMsg = tr("Kunde inte skapa katalogen: %1").arg(name);
    }
    
    // Rapportera resultatet i GUI-tråden
    QMetaObject::invokeMethod(this, "createDirectoryResult", Qt::QueuedConnection,
                             Q_ARG(bool, success), Q_ARG(QString, errorMsg));
}

void FileModel::createDirectoryResult(bool success, const QString &errorMsg)
{
    if (!success) {
        emit error(errorMsg);
    } else {
        refresh();
    }
}

bool FileModel::deletePath(const QString &path)
{
    FileSystemTask* task = new FileSystemTask(this, 
                                            FileSystemTask::DeleteFile, 
                                            path);
    QThreadPool::globalInstance()->start(task);
    
    return true;
}

void FileModel::deletePathTask(const QString &path)
{
    bool success = false;
    QString errorMsg;
    
    QFileInfo fileInfo(path);
    if (fileInfo.isDir()) {
        QDir dir(path);
        success = dir.removeRecursively();
        if (!success) {
            errorMsg = tr("Kunde inte radera katalogen: %1").arg(path);
        }
    } else {
        QFile file(path);
        success = file.remove();
        if (!success) {
            errorMsg = tr("Kunde inte radera filen: %1").arg(path);
        }
    }
    
    // Rapportera resultatet i GUI-tråden
    QMetaObject::invokeMethod(this, "deletePathResult", Qt::QueuedConnection,
                             Q_ARG(bool, success), Q_ARG(QString, errorMsg));
}

void FileModel::deletePathResult(bool success, const QString &errorMsg)
{
    if (!success) {
        emit error(errorMsg);
    } else {
        refresh();
    }
}

bool FileModel::renamePath(const QString &oldPath, const QString &newName)
{
    QFileInfo oldInfo(oldPath);
    QString newPath = oldInfo.absolutePath() + "/" + newName;
    
    FileSystemTask* task = new FileSystemTask(this, 
                                            FileSystemTask::RenameFile, 
                                            oldPath, newPath);
    QThreadPool::globalInstance()->start(task);
    
    return true;
}

void FileModel::renamePathTask(const QString &oldPath, const QString &newPath)
{
    bool success = false;
    QString errorMsg;
    
    QFile file(oldPath);
    success = file.rename(newPath);
    
    if (!success) {
        errorMsg = tr("Kunde inte byta namn på: %1").arg(oldPath);
    }
    
    // Rapportera resultatet i GUI-tråden
    QMetaObject::invokeMethod(this, "renamePathResult", Qt::QueuedConnection,
                             Q_ARG(bool, success), Q_ARG(QString, errorMsg));
}

void FileModel::renamePathResult(bool success, const QString &errorMsg)
{
    if (!success) {
        emit error(errorMsg);
    } else {
        refresh();
    }
}

QVariantMap FileModel::get(int index) const
{
    QVariantMap result;
    if (index < 0 || index >= m_files.size())
        return result;
    
    const FileInfo &info = m_files[index];
    
    result["fileName"] = info.fileName;
    result["filePath"] = info.filePath;
    result["fileSize"] = formatFileSize(info.fileSize);
    result["fileDate"] = info.fileDate.toString("yyyy-MM-dd hh:mm");
    result["isDirectory"] = info.isDirectory;
    
    return result;
}

QString FileModel::currentPath() const
{
    return m_currentPath;
}

void FileModel::setCurrentPath(const QString &path)
{
    if (m_currentPath != path) {
        navigate(path);
    }
}

bool FileModel::isRemote() const
{
    return m_isRemote;
}

bool FileModel::isLoading() const
{
    return m_isLoading;
}

bool FileModel::hasMoreItems() const
{
    return m_hasMoreItems;
}

QString FileModel::formatFileSize(qint64 size) const
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    
    if (size < KB) {
        return QString("%1 B").arg(size);
    } else if (size < MB) {
        return QString("%1 KB").arg(size / (double)KB, 0, 'f', 1);
    } else if (size < GB) {
        return QString("%1 MB").arg(size / (double)MB, 0, 'f', 1);
    } else {
        return QString("%1 GB").arg(size / (double)GB, 0, 'f', 1);
    }
} 