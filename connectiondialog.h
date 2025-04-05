#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QDialog>
#include <QList>
#include "connection.h"

namespace Ui {
class ConnectionDialog;
}

/**
 * @brief Dialog för att hantera FTP/SFTP-anslutningar
 * 
 * Denna dialog låter användaren skapa, redigera och radera anslutningar
 * samt välja en anslutning att ansluta till.
 */
class ConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor
     * @param parent Förälder-widget
     */
    explicit ConnectionDialog(QWidget *parent = nullptr);
    
    /**
     * @brief Destruktor
     */
    ~ConnectionDialog();
    
    /**
     * @brief Sätt listan med anslutningar
     * @param connections Lista med anslutningar att visa
     */
    void setConnections(const QList<Connection> &connections);
    
    /**
     * @brief Hämta listan med anslutningar
     * @return Lista med anslutningar
     */
    QList<Connection> connections() const;
    
    /**
     * @brief Hämta den valda anslutningen
     * @return Den valda anslutningen eller en tom anslutning om ingen är vald
     */
    Connection selectedConnection() const;

private slots:
    /**
     * @brief Hantera klick på "Lägg till"-knappen
     */
    void onAddButtonClicked();
    
    /**
     * @brief Hantera klick på "Redigera"-knappen
     */
    void onEditButtonClicked();
    
    /**
     * @brief Hantera klick på "Radera"-knappen
     */
    void onDeleteButtonClicked();
    
    /**
     * @brief Hantera dubbelklick på en anslutning i listan
     */
    void onConnectionDoubleClicked(const QModelIndex &index);
    
    /**
     * @brief Hantera när en anslutning väljs i listan
     */
    void onConnectionSelectionChanged();
    
    /**
     * @brief Hantera klick på "Anslut"-knappen
     */
    void onConnectButtonClicked();
    
    /**
     * @brief Hantera protokollbyte
     */
    void onProtocolChanged(int index);

private:
    /**
     * @brief Uppdatera anslutningslistan i UI
     */
    void updateConnectionList();
    
    /**
     * @brief Öppna dialogrutan för att lägga till/redigera en anslutning
     * @param connection Anslutning att redigera eller tom för att skapa en ny
     * @return true om anslutningen sparades, annars false
     */
    bool openConnectionEditor(Connection &connection);
    
    Ui::ConnectionDialog *ui;
    QList<Connection> m_connections;
    int m_selectedIndex;
};

#endif // CONNECTIONDIALOG_H 