#ifndef CONNECTION_H
#define CONNECTION_H

#include <QString>

/**
 * @brief Struktur som håller anslutningsinformation för FTP/SFTP-servrar
 */
class Connection
{
public:
    /**
     * @brief Protokolltyper som stöds
     */
    enum Protocol {
        FTP,
        SFTP
    };
    
    /**
     * @brief Autentiseringsmetod för anslutning
     */
    enum AuthMethod {
        PASSWORD,   ///< Lösenordsautentisering
        KEY,        ///< SSH-nyckel
        BOTH        ///< Både lösenord och nyckel
    };
    
    /**
     * @brief Standardkonstruktor
     */
    Connection()
        : protocol(FTP)
        , port(0)
        , savePassword(false) // Default to false
        , authMethod(PASSWORD) // Default to PASSWORD
        , privateKeyPath("")   // Initialize new field
        , keyPassphrase("")    // Initialize new field
    {
    }
    
    /**
     * @brief Konstruktor med parametrar
     */
    Connection(const QString &name, Protocol protocol, const QString &host, 
               quint16 port, const QString &username, const QString &password,
               bool savePasswordValue) // Renamed to avoid conflict with member
        : name(name)
        , protocol(protocol)
        , host(host)
        , port(port)
        , username(username)
        , password(password)
        , savePassword(savePasswordValue) // Use parameter
        , authMethod(PASSWORD)      // Default to PASSWORD
        , privateKeyPath("")        // Initialize new field
        , keyPassphrase("")         // Initialize new field
    {
    }
    
    /**
     * @brief Konstruktor med parametrar inklusive SSH-nyckel
     */
    Connection(const QString &name, Protocol protocol, const QString &host, 
               quint16 port, const QString &username, const QString &password,
               bool savePasswordValue, AuthMethod authMethodValue, // Renamed to avoid conflict
               const QString &privateKeyPathValue, const QString &keyPassphraseValue) // Renamed
        : name(name)
        , protocol(protocol)
        , host(host)
        , port(port)
        , username(username)
        , password(password)
        , savePassword(savePasswordValue) // Use parameter
        , authMethod(authMethodValue)   // Use parameter
        , privateKeyPath(privateKeyPathValue) // Use parameter
        , keyPassphrase(keyPassphraseValue)   // Use parameter
    {
    }

    // Helper methods from src/app/connection.h
    bool isPasswordAuthRequired() const {
        if (protocol == FTP) return true;
        if (protocol == SFTP && (authMethod == PASSWORD || authMethod == BOTH)) return true;
        return false;
    }

    bool isKeyAuthRequired() const {
        if (protocol == SFTP && (authMethod == KEY || authMethod == BOTH) && !privateKeyPath.isEmpty()) return true;
        return false;
    }
    
    /**
     * @brief Hämta standardportnummer baserat på protokollet
     * @return Standardport för det valda protokollet
     */
    static quint16 defaultPort(Protocol protocol)
    {
        switch (protocol) {
            case FTP: return 21;
            case SFTP: return 22;
            default: return 0;
        }
    }
    
    /**
     * @brief Konvertera protokoll till strängrepresentation
     * @param protocol Protokollet
     * @return Strängrepresentation av protokollet (t.ex. "FTP", "SFTP")
     */
    static QString protocolToString(Protocol protocol)
    {
        switch (protocol) {
            case FTP: return "FTP";
            case SFTP: return "SFTP";
            default: return QString();
        }
    }
    
    /**
     * @brief Konvertera strängrepresentation till protokoll
     * @param str Strängrepresentation av protokollet
     * @return Protokollet eller FTP om strängen inte matchar något känt protokoll
     */
    static Protocol stringToProtocol(const QString &str)
    {
        if (str.toUpper() == "SFTP") {
            return SFTP;
        }
        return FTP;  // Default
    }
    
    QString name;            ///< Anslutningsnamn för användaren
    Protocol protocol;       ///< Anslutningsprotokoll (FTP eller SFTP)
    QString host;            ///< Värddatorns namn eller IP-adress
    quint16 port;            ///< Portnummer
    QString username;        ///< Användarnamn för inloggning
    QString password;        ///< Lösenord för inloggning
    // bool savePassword;    ///< Om lösenordet ska sparas - already exists, ensure it's used correctly
    AuthMethod authMethod;   ///< Autentiseringsmetod
    // QString privateKeyPath;  ///< Sökväg till privat SSH-nyckel (för SFTP) - already exists
    // QString keyPassphrase;   ///< Lösenfras för SSH-nyckeln (om den är krypterad) - already exists
};

#endif // CONNECTION_H