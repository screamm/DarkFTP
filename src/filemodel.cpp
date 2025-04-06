#include "filemodel.h"
#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QDir>
#include <QMessageBox> // För framtida bekräftelsedialoger kanske

FileModel::FileModel(bool remote, QObject *parent)
    : QAbstractListModel(parent), m_isRemote(remote)
{
    // Definiera rollnamnen som QML kan använda
    m_roleNames[FileNameRole] = "fileName";
    m_roleNames[FilePathRole] = "filePath";
    m_roleNames[FileSizeRole] = "fileSize";
    m_roleNames[FileDateRole] = "fileDate";
    m_roleNames[IsDirectoryRole] = "isDirectory";

    // Sätt en initial sökväg (t.ex. användarens hemkatalog eller roten)
    setCurrentPath(QDir::homePath());
}

int FileModel::rowCount(const QModelIndex &parent) const
{
    // För enkla listmodeller är föräldern alltid ogiltig.
    if (parent.isValid())
        return 0;
    return m_files.count();
}

QVariant FileModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_files.count())
        return QVariant();

    const FileInfo &file = m_files.at(index.row());

    switch (role) {
    case FileNameRole:
        return file.fileName;
    case FilePathRole:
        return file.filePath;
    case FileSizeRole:
        // Formatera storleken för läsbarhet
        if (file.isDirectory) return QStringLiteral("--"); // Ingen storlek för mappar
        if (file.fileSize < 1024) return QString::number(file.fileSize) + " B";
        if (file.fileSize < 1024 * 1024) return QString::number(file.fileSize / 1024.0, 'f', 1) + " KB";
        if (file.fileSize < 1024 * 1024 * 1024) return QString::number(file.fileSize / (1024.0 * 1024.0), 'f', 1) + " MB";
        return QString::number(file.fileSize / (1024.0 * 1024.0 * 1024.0), 'f', 1) + " GB";
    case FileDateRole:
        return file.fileDate.toString("yyyy-MM-dd hh:mm");
    case IsDirectoryRole:
        return file.isDirectory;
    default:
        break;
    }

    return QVariant();
}

QHash<int, QByteArray> FileModel::roleNames() const
{
    return m_roleNames;
}

void FileModel::navigate(const QString &path)
{
    setCurrentPath(path);
}

void FileModel::refresh()
{
    listDirectory(m_currentPath);
}

void FileModel::goUp()
{
    QDir dir(m_currentPath);
    if (dir.cdUp()) {
        setCurrentPath(dir.absolutePath());
    } else {
        qWarning() << "Kan inte gå upp från:" << m_currentPath;
        emit error(QString("Kan inte gå uppåt från '%1'.").arg(m_currentPath));
    }
}

QString FileModel::currentPath() const
{
    return m_currentPath;
}

void FileModel::setCurrentPath(const QString &path)
{
    QString cleanedPath = QDir::cleanPath(path);
    // Säkerställ att sökvägen är giltig (existerar och är en katalog)
    QFileInfo pathInfo(cleanedPath);
    if (!pathInfo.exists() || !pathInfo.isDir()) {
        qWarning() << "Ogiltig sökväg:" << cleanedPath;
        // Försök gå till roten om den angivna sökvägen är dålig?
        // Eller sänd ett fel till QML?
        // För nu: Gå till hemkatalogen som fallback
        cleanedPath = QDir::homePath(); 
    }

    if (m_currentPath != cleanedPath) {
        m_currentPath = cleanedPath;
        emit currentPathChanged(m_currentPath);
        listDirectory(m_currentPath);
    }
}

bool FileModel::isRemote() const
{
    return m_isRemote;
}

// === Nya metoder för filhantering ===

bool FileModel::createDirectory(const QString &name)
{
    if (m_isRemote) {
        emit error("Skapa mapp är inte implementerat för fjärrvärd än.");
        return false;
    }
    if (name.isEmpty() || name.contains('/') || name.contains('\\')) {
         emit error(QString("Ogiltigt mappnamn: '%1'.").arg(name));
         return false;
    }

    QDir currentDir(m_currentPath);
    QString newDirPath = currentDir.filePath(name);

    if (currentDir.exists(name)) {
        emit error(QString("En fil eller mapp med namnet '%1' finns redan.").arg(name));
        return false;
    }

    if (currentDir.mkdir(name)) {
        qInfo() << "Skapade mapp:" << newDirPath;
        refresh(); // Uppdatera vyn
        return true;
    } else {
        emit error(QString("Kunde inte skapa mappen '%1'. Kontrollera rättigheter.").arg(newDirPath));
        return false;
    }
}

bool FileModel::deletePath(const QString &path)
{
    if (m_isRemote) {
        emit error("Ta bort är inte implementerat för fjärrvärd än.");
        return false;
    }
    if (path.isNull() || path.isEmpty()) {
        emit error("Ogiltig sökväg för borttagning.");
        return false;
    }

    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        emit error(QString("Filen eller mappen '%1' finns inte.").arg(path));
        return false;
    }

    bool success = false;
    QString errorMsg;

    // Här borde man ha en bekräftelsedialog!
    // QMessageBox::StandardButton reply;
    // reply = QMessageBox::question(nullptr, "Bekräfta borttagning",
    //                              QString("Vill du verkligen ta bort '%1'?").arg(fileInfo.fileName()),
    //                              QMessageBox::Yes|QMessageBox::No);
    // if (reply == QMessageBox::No) {
    //     return false;
    // }

    if (fileInfo.isDir()) {
        QDir dir(path);
        // Viktigt: Ta bort rekursivt!
        success = dir.removeRecursively();
        if (!success) {
            errorMsg = QString("Kunde inte ta bort mappen '%1'. Är den tom? Har du rättigheter?").arg(path);
        }
    } else if (fileInfo.isFile()) {
        QFile file(path);
        success = file.remove();
        if (!success) {
            errorMsg = QString("Kunde inte ta bort filen '%1'. Har du rättigheter?").arg(path);
        }
    } else {
        errorMsg = QString("Okänd typ av sökväg: '%1'.").arg(path);
    }

    if (success) {
        qInfo() << "Tog bort:" << path;
        refresh(); // Uppdatera vyn
    } else {
        emit error(errorMsg);
    }
    return success;
}

bool FileModel::renamePath(const QString &oldPath, const QString &newName)
{
    if (m_isRemote) {
        emit error("Döp om är inte implementerat för fjärrvärd än.");
        return false;
    }
    if (oldPath.isNull() || oldPath.isEmpty()) {
        emit error("Ogiltig källsökväg för namnbyte.");
        return false;
    }
    if (newName.isEmpty() || newName.contains('/') || newName.contains('\\')) {
         emit error(QString("Ogiltigt nytt namn: '%1'.").arg(newName));
         return false;
    }

    QFileInfo oldInfo(oldPath);
    if (!oldInfo.exists()) {
        emit error(QString("Källan '%1' finns inte.").arg(oldPath));
        return false;
    }

    QDir parentDir = oldInfo.dir(); // Hämta föräldrakatalogen
    QString newPath = parentDir.filePath(newName);

    if (QFileInfo::exists(newPath)) {
        emit error(QString("En fil eller mapp med namnet '%1' finns redan.").arg(newName));
        return false;
    }

    bool success = false;
    if (oldInfo.isDir()) {
        QDir dir;
        success = dir.rename(oldPath, newPath);
    } else if (oldInfo.isFile()) {
        QFile file;
        success = file.rename(oldPath, newPath);
    } else {
         emit error(QString("Okänd typ av sökväg: '%1'.").arg(oldPath));
         return false;
    }

    if (success) {
        qInfo() << "Döpte om" << oldPath << "till" << newPath;
        refresh(); // Uppdatera vyn
        return true;
    } else {
        emit error(QString("Kunde inte döpa om '%1' till '%2'. Kontrollera rättigheter.").arg(oldPath).arg(newPath));
        return false;
    }
}

// Privat metod för att faktiskt lista katalogen
void FileModel::listDirectory(const QString &path)
{
    beginResetModel(); // Informera vyn att modellen kommer att ändras helt
    m_files.clear();

    QDir dir(path);
    if (!dir.exists()) {
        emit error(QString("Katalogen '%1' finns inte.").arg(path));
        endResetModel();
        return;
    }

    // Lista alla filer och kataloger, inga punkter (.), inga dolda filer som standard
    // Senare kan detta göras konfigurerbart
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable);
    dir.setSorting(QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);

    QFileInfoList list = dir.entryInfoList();
    for (const QFileInfo &fileInfo : list) {
        FileInfo file;
        file.fileName = fileInfo.fileName();
        file.filePath = fileInfo.absoluteFilePath(); // Använd absolut sökväg
        file.fileSize = fileInfo.size();
        file.fileDate = fileInfo.lastModified();
        file.isDirectory = fileInfo.isDir();
        m_files.append(file);
    }

    endResetModel(); // Informera vyn att modellen har uppdaterats
} 