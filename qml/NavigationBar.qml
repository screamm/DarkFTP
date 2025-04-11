import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: navigationBar
    height: 50
    color: theme.panel
    border.width: 1
    border.color: theme.panelBorder
    
    // Tillgång till tema och sparade servrar från applikationsfönstret
    property var theme: mainWindow.theme
    property var savedServers: mainWindow.savedServers
    
    // Signal för att öppna anslutningsdialogrutan
    signal connectRequested()
    
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 15
        anchors.rightMargin: 15
        spacing: 15
        
        // Logotyp/programnamn
        Text {
            text: "DarkFTP"
            font.pixelSize: 22
            font.bold: true
            color: theme.accent
        }
        
        // Dropdown för sparade servrar
        ComboBox {
            id: serverComboBox
            Layout.preferredWidth: 180
            Layout.preferredHeight: 36
            model: savedServers
            textRole: "name"
            
            background: Rectangle {
                implicitWidth: 180
                implicitHeight: 36
                radius: 18
                color: theme.listItem
                border.width: 1
                border.color: theme.panelBorder
            }
            
            contentItem: Text {
                leftPadding: 12
                rightPadding: 12
                text: serverComboBox.displayText
                font.pixelSize: 14
                color: theme.text
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            
            delegate: ItemDelegate {
                width: serverComboBox.width
                contentItem: Text {
                    text: modelData.name
                    color: theme.text
                    font.pixelSize: 14
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                }
                highlighted: serverComboBox.highlightedIndex === index
                
                background: Rectangle {
                    color: highlighted ? theme.accent : "transparent"
                    opacity: highlighted ? 0.3 : 1.0
                }
            }
            
            popup: Popup {
                y: serverComboBox.height
                width: serverComboBox.width
                implicitHeight: contentItem.implicitHeight
                padding: 1
                
                contentItem: ListView {
                    clip: true
                    implicitHeight: contentHeight
                    model: serverComboBox.popup.visible ? serverComboBox.model : null
                    
                    delegate: ItemDelegate {
                        width: serverComboBox.width
                        
                        contentItem: Text {
                            text: modelData.name
                            color: theme.text
                            font.pixelSize: 14
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                        }
                        
                        highlighted: serverComboBox.highlightedIndex === index
                        
                        background: Rectangle {
                            color: highlighted ? theme.accent : "transparent"
                            opacity: highlighted ? 0.3 : 1.0
                        }
                    }
                    
                    ScrollIndicator.vertical: ScrollIndicator { }
                }
                
                background: Rectangle {
                    color: theme.panel
                    border.color: theme.panelBorder
                    border.width: 1
                    radius: 5
                }
            }
        }
        
        // Anslutningsknapp
        Button {
            text: "Anslut"
            Layout.preferredWidth: 100
            Layout.preferredHeight: 36
            
            contentItem: Text {
                text: parent.text
                font.pixelSize: 14
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            
            background: Rectangle {
                radius: 18
                color: parent.down ? Qt.darker(theme.accent, 1.3) : 
                       parent.hovered ? Qt.darker(theme.accent, 1.1) : theme.accent
            }
            
            onClicked: {
                if (serverComboBox.currentIndex >= 0) {
                    var selectedServer = savedServers[serverComboBox.currentIndex];
                    console.log("Ansluter till sparad server: " + selectedServer.name);
                    
                    // Hämta autentiseringsmetod och värden från den sparade servern
                    var authMethod = selectedServer.authMethod !== undefined ? selectedServer.authMethod : 0; // Default PASSWORD
                    var keyPath = selectedServer.privateKeyPath !== undefined ? selectedServer.privateKeyPath : "";
                    var keyPassphrase = selectedServer.keyPassphrase !== undefined ? selectedServer.keyPassphrase : "";
                    
                    // Anropa backend med information från den sparade servern och nyckelinformation
                    backend.connectFromQmlEx(
                        selectedServer.protocol, 
                        selectedServer.host, 
                        selectedServer.port, 
                        selectedServer.username, 
                        selectedServer.password,
                        authMethod,
                        keyPath,
                        keyPassphrase
                    );
                } else {
                    // Öppna anslutningsdialogrutan om ingen server är vald
                    console.log("Ingen server vald, öppnar anslutningsdialog.");
                    navigationBar.connectRequested();
                }
            }
        }
        
        // Adressfält för fjärrsökväg
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            radius: 18
            color: theme.listItem
            border.width: 1
            border.color: theme.panelBorder
            
            TextInput {
                id: remotePathInput
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                verticalAlignment: Text.AlignVCenter
                color: theme.text
                font.pixelSize: 14
                selectByMouse: true
                
                // Placeholder text
                Text {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    verticalAlignment: Text.AlignVCenter
                    text: "Fjärrsökväg"
                    color: theme.text
                    font.pixelSize: 14
                    opacity: 0.5
                    visible: !remotePathInput.text && !remotePathInput.activeFocus
                }
                
                onAccepted: {
                    console.log("Byter fjärrkatalog till: " + text);
                }
            }
        }
        
        // Knapp för att skapa ny anslutning
        Button {
            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            
            contentItem: Text {
                text: "+"
                font.pixelSize: 18
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            
            background: Rectangle {
                radius: 18
                color: parent.down ? Qt.darker(theme.accent, 1.3) : 
                       parent.hovered ? Qt.darker(theme.accent, 1.1) : theme.accent
            }
            
            onClicked: {
                navigationBar.connectRequested();
            }
        }
        
        // Inställningsknapp
        Button {
            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            
            contentItem: Text {
                text: "⚙"
                font.pixelSize: 18
                color: theme.text
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            
            background: Rectangle {
                radius: 18
                color: parent.down ? Qt.darker(theme.listItem, 1.3) : 
                       parent.hovered ? Qt.darker(theme.listItem, 1.1) : theme.listItem
                border.width: 1
                border.color: theme.panelBorder
            }
            
            onClicked: {
                console.log("Öppnar inställningar");
                // Här kan vi implementera inställningsdialogrutan
            }
        }
    }
} 