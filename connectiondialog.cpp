#include "connectiondialog.h"
#include "ui_connectiondialog.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QSettings>

ConnectionDialog::ConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionDialog),
    m_selectedIndex(-1)
{
    ui->setupUi(this);
    
    // Anslut signaler och slots
    connect(ui->addButton, &QPushButton::clicked, this, &ConnectionDialog::onAddButtonClicked);
    connect(ui->editButton, &QPushButton::clicked, this, &ConnectionDialog::onEditButtonClicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &ConnectionDialog::onDeleteButtonClicked);
    connect(ui->connectButton, &QPushButton::clicked, this, &ConnectionDialog::onConnectButtonClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &ConnectionDialog::reject);
    connect(ui->connectionListWidget, &QListWidget::itemSelectionChanged, this, &ConnectionDialog::onConnectionSelectionChanged);
    connect(ui->connectionListWidget, &QListWidget::doubleClicked, this, &ConnectionDialog::onConnectionDoubleClicked);
    connect(ui->protocolComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ConnectionDialog::onProtocolChanged);
    
    // Ladda sparade anslutningar
    QSettings settings;
    int size = settings.beginReadArray("connections");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        Connection conn;
        conn.name = settings.value("name").toString();
        conn.protocol = Connection::stringToProtocol(settings.value("protocol").toString());
        conn.host = settings.value("host").toString();
        conn.port = settings.value("port", Connection::defaultPort(conn.protocol)).toUInt();
        conn.username = settings.value("username").toString();
        if (settings.value("savePassword", false).toBool()) {
            conn.password = settings.value("password").toString();
            conn.savePassword = true;
        } else {
            conn.savePassword = false;
        }
        m_connections.append(conn);
    }
    settings.endArray();
    
    // Uppdatera anslutningslistan
    updateConnectionList();
    onConnectionSelectionChanged();
}

ConnectionDialog::~ConnectionDialog()
{
    // Spara anslutningar innan dialogrutan stängs
    QSettings settings;
    settings.beginWriteArray("connections");
    for (int i = 0; i < m_connections.size(); ++i) {
        settings.setArrayIndex(i);
        const Connection &conn = m_connections.at(i);
        settings.setValue("name", conn.name);
        settings.setValue("protocol", Connection::protocolToString(conn.protocol));
        settings.setValue("host", conn.host);
        settings.setValue("port", conn.port);
        settings.setValue("username", conn.username);
        settings.setValue("savePassword", conn.savePassword);
        if (conn.savePassword) {
            settings.setValue("password", conn.password);
        } else {
            settings.remove("password");
        }
    }
    settings.endArray();
    
    delete ui;
}

void ConnectionDialog::setConnections(const QList<Connection> &connections)
{
    m_connections = connections;
    updateConnectionList();
}

QList<Connection> ConnectionDialog::connections() const
{
    return m_connections;
}

Connection ConnectionDialog::selectedConnection() const
{
    if (m_selectedIndex >= 0 && m_selectedIndex < m_connections.size()) {
        return m_connections.at(m_selectedIndex);
    }
    return Connection();
}

void ConnectionDialog::onAddButtonClicked()
{
    Connection newConnection;
    if (openConnectionEditor(newConnection)) {
        m_connections.append(newConnection);
        updateConnectionList();
        
        // Välj den nya anslutningen
        ui->connectionListWidget->setCurrentRow(m_connections.size() - 1);
    }
}

void ConnectionDialog::onEditButtonClicked()
{
    if (m_selectedIndex >= 0 && m_selectedIndex < m_connections.size()) {
        Connection conn = m_connections.at(m_selectedIndex);
        if (openConnectionEditor(conn)) {
            m_connections[m_selectedIndex] = conn;
            updateConnectionList();
            
            // Återställ valet
            ui->connectionListWidget->setCurrentRow(m_selectedIndex);
        }
    }
}

void ConnectionDialog::onDeleteButtonClicked()
{
    if (m_selectedIndex >= 0 && m_selectedIndex < m_connections.size()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, tr("Radera anslutning"),
            tr("Är du säker på att du vill radera anslutningen '%1'?").arg(m_connections.at(m_selectedIndex).name),
            QMessageBox::Yes | QMessageBox::No
        );
        
        if (reply == QMessageBox::Yes) {
            m_connections.removeAt(m_selectedIndex);
            updateConnectionList();
            onConnectionSelectionChanged();
        }
    }
}

void ConnectionDialog::onConnectionDoubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    onConnectButtonClicked();
}

void ConnectionDialog::onConnectionSelectionChanged()
{
    int row = ui->connectionListWidget->currentRow();
    m_selectedIndex = row;
    
    bool hasSelection = (row >= 0);
    ui->editButton->setEnabled(hasSelection);
    ui->deleteButton->setEnabled(hasSelection);
    ui->connectButton->setEnabled(hasSelection);
    
    if (hasSelection && row < m_connections.size()) {
        const Connection &conn = m_connections.at(row);
        
        // Visa anslutningsinformationen i detaljvyn
        if (conn.protocol == Connection::FTP) {
            ui->protocolComboBox->setCurrentIndex(0);
        } else {
            ui->protocolComboBox->setCurrentIndex(1);
        }
        
        ui->hostLineEdit->setText(conn.host);
        ui->portSpinBox->setValue(conn.port);
        ui->usernameLineEdit->setText(conn.username);
        ui->passwordLineEdit->setText(conn.password);
        ui->savePasswordCheckBox->setChecked(conn.savePassword);
    } else {
        // Rensa detaljvyn
        ui->protocolComboBox->setCurrentIndex(0);
        ui->hostLineEdit->clear();
        ui->portSpinBox->setValue(Connection::defaultPort(Connection::FTP));
        ui->usernameLineEdit->clear();
        ui->passwordLineEdit->clear();
        ui->savePasswordCheckBox->setChecked(false);
    }
}

void ConnectionDialog::onConnectButtonClicked()
{
    if (m_selectedIndex >= 0 && m_selectedIndex < m_connections.size()) {
        // Om lösenordet inte är sparat, fråga användaren efter det
        Connection &conn = m_connections[m_selectedIndex];
        if (!conn.savePassword || conn.password.isEmpty()) {
            bool ok;
            QString password = QInputDialog::getText(
                this, tr("Ange lösenord"),
                tr("Lösenord för '%1':").arg(conn.username),
                QLineEdit::Password, QString(), &ok
            );
            
            if (!ok) {
                return;
            }
            
            conn.password = password;
        }
        
        accept();
    }
}

void ConnectionDialog::onProtocolChanged(int index)
{
    Connection::Protocol protocol = (index == 1) ? Connection::SFTP : Connection::FTP;
    ui->portSpinBox->setValue(Connection::defaultPort(protocol));
}

void ConnectionDialog::updateConnectionList()
{
    ui->connectionListWidget->clear();
    
    for (const Connection &conn : m_connections) {
        QString displayText = QString("%1 - %2@%3:%4 (%5)")
            .arg(conn.name)
            .arg(conn.username)
            .arg(conn.host)
            .arg(conn.port)
            .arg(Connection::protocolToString(conn.protocol));
        
        ui->connectionListWidget->addItem(displayText);
    }
}

bool ConnectionDialog::openConnectionEditor(Connection &connection)
{
    // Skapa en dialog för att redigera anslutningen
    QDialog dialog(this);
    dialog.setWindowTitle(connection.name.isEmpty() ? tr("Lägg till anslutning") : tr("Redigera anslutning"));
    
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    
    // Formulär för anslutningsuppgifter
    QFormLayout *formLayout = new QFormLayout();
    
    QLineEdit *nameEdit = new QLineEdit(connection.name);
    formLayout->addRow(tr("Namn:"), nameEdit);
    
    QComboBox *protocolCombo = new QComboBox();
    protocolCombo->addItem(tr("FTP"));
    protocolCombo->addItem(tr("SFTP"));
    protocolCombo->setCurrentIndex(connection.protocol == Connection::SFTP ? 1 : 0);
    formLayout->addRow(tr("Protokoll:"), protocolCombo);
    
    QLineEdit *hostEdit = new QLineEdit(connection.host);
    formLayout->addRow(tr("Värd:"), hostEdit);
    
    QSpinBox *portSpin = new QSpinBox();
    portSpin->setRange(1, 65535);
    portSpin->setValue(connection.port > 0 ? connection.port : Connection::defaultPort(connection.protocol));
    formLayout->addRow(tr("Port:"), portSpin);
    
    QLineEdit *usernameEdit = new QLineEdit(connection.username);
    formLayout->addRow(tr("Användarnamn:"), usernameEdit);
    
    QLineEdit *passwordEdit = new QLineEdit(connection.password);
    passwordEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow(tr("Lösenord:"), passwordEdit);
    
    QCheckBox *savePasswordCheck = new QCheckBox();
    savePasswordCheck->setChecked(connection.savePassword);
    formLayout->addRow(tr("Spara lösenord:"), savePasswordCheck);
    
    layout->addLayout(formLayout);
    
    // Knapparna
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttonBox);
    
    // Uppdatera port när protokoll ändras
    connect(protocolCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        Connection::Protocol protocol = (index == 1) ? Connection::SFTP : Connection::FTP;
        if (portSpin->value() == Connection::defaultPort(connection.protocol)) {
            portSpin->setValue(Connection::defaultPort(protocol));
        }
    });
    
    // Kör dialogen
    if (dialog.exec() == QDialog::Accepted) {
        // Samla in de nya värdena
        connection.name = nameEdit->text();
        connection.protocol = (protocolCombo->currentIndex() == 1) ? Connection::SFTP : Connection::FTP;
        connection.host = hostEdit->text();
        connection.port = portSpin->value();
        connection.username = usernameEdit->text();
        connection.password = passwordEdit->text();
        connection.savePassword = savePasswordCheck->isChecked();
        
        // Kontrollera att nödvändiga fält är ifyllda
        if (connection.name.isEmpty() || connection.host.isEmpty() || connection.username.isEmpty()) {
            QMessageBox::warning(this, tr("Saknade uppgifter"), 
                tr("Vänligen fyll i namn, värd och användarnamn för anslutningen."));
            return false;
        }
        
        return true;
    }
    
    return false;
} 