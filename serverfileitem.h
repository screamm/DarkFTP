#ifndef SERVERFILEITEM_H
#define SERVERFILEITEM_H

#include <QString>
#include <QDateTime>

/**
 * @brief Klass som representerar ett fil- eller katalogobjekt på en FTP/SFTP-server
 */
class ServerFileItem
{
public:
    /**
     * @brief Standardkonstruktor
     */
    ServerFileItem() : m_isDirectory(false), m_size(0) {}
    
    /**
     * @brief Konstruktor med parametrar
     * @param name Filnamn
     * @param isDirectory Om det är en katalog
     * @param size Filstorlek i bytes
     * @param permissions Rättighetssträng (t.ex. "drwxr-xr-x")
     * @param lastModified Senast ändrad (datum och tid)
     */
    ServerFileItem(const QString &name, bool isDirectory, qint64 size,
                   const QString &permissions, const QDateTime &lastModified)
        : m_name(name)
        , m_isDirectory(isDirectory)
        , m_size(size)
        , m_permissions(permissions)
        , m_lastModified(lastModified)
    {
    }
    
    /**
     * @brief Hämta filnamnet
     * @return Filnamnet
     */
    QString name() const { return m_name; }
    
    /**
     * @brief Sätt filnamnet
     * @param name Filnamnet
     */
    void setName(const QString &name) { m_name = name; }
    
    /**
     * @brief Kontrollera om det är en katalog
     * @return true om det är en katalog, annars false
     */
    bool isDirectory() const { return m_isDirectory; }
    
    /**
     * @brief Sätt om det är en katalog
     * @param isDirectory true om det är en katalog, annars false
     */
    void setIsDirectory(bool isDirectory) { m_isDirectory = isDirectory; }
    
    /**
     * @brief Hämta filstorleken
     * @return Filstorleken i bytes
     */
    qint64 size() const { return m_size; }
    
    /**
     * @brief Sätt filstorleken
     * @param size Filstorleken i bytes
     */
    void setSize(qint64 size) { m_size = size; }
    
    /**
     * @brief Hämta rättigheterna
     * @return Rättighetssträng (t.ex. "drwxr-xr-x")
     */
    QString permissions() const { return m_permissions; }
    
    /**
     * @brief Sätt rättigheterna
     * @param permissions Rättighetssträng (t.ex. "drwxr-xr-x")
     */
    void setPermissions(const QString &permissions) { m_permissions = permissions; }
    
    /**
     * @brief Hämta senast ändrad
     * @return Senast ändrad (datum och tid)
     */
    QDateTime lastModified() const { return m_lastModified; }
    
    /**
     * @brief Sätt senast ändrad
     * @param lastModified Senast ändrad (datum och tid)
     */
    void setLastModified(const QDateTime &lastModified) { m_lastModified = lastModified; }
    
    /**
     * @brief Hämta filändelsen
     * @return Filändelsen utan punkt (t.ex. "txt" för fil.txt)
     */
    QString extension() const {
        if (m_isDirectory) {
            return QString();
        }
        
        int lastDot = m_name.lastIndexOf('.');
        if (lastDot > 0) {
            return m_name.mid(lastDot + 1).toLower();
        }
        
        return QString();
    }
    
    /**
     * @brief Formatera filstorleken till läsbar form
     * @return Formaterad filstorlek (t.ex. "1.2 MB")
     */
    QString formattedSize() const {
        if (m_isDirectory) {
            return QString();
        }
        
        constexpr qint64 KB = 1024;
        constexpr qint64 MB = KB * 1024;
        constexpr qint64 GB = MB * 1024;
        
        if (m_size < KB) {
            return QString("%1 B").arg(m_size);
        } else if (m_size < MB) {
            return QString("%1 KB").arg(m_size / (double)KB, 0, 'f', 1);
        } else if (m_size < GB) {
            return QString("%1 MB").arg(m_size / (double)MB, 0, 'f', 1);
        } else {
            return QString("%1 GB").arg(m_size / (double)GB, 0, 'f', 1);
        }
    }
    
private:
    QString m_name;
    bool m_isDirectory;
    qint64 m_size;
    QString m_permissions;
    QDateTime m_lastModified;
};

#endif // SERVERFILEITEM_H 