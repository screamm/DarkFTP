import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "." as DarkFTP

Popup {
    id: connectionDialog
    width: 500
    height: 500
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    
    // Tillgång till tema från applikationsfönstret
    property var theme: mainWindow.theme
    
    // Bakgrundseffekt när dialogrutan visas
    background: Rectangle {
        color: "transparent"
    }
    
    // Bakgrundsdimning
    Rectangle {
        anchors.fill: parent
        color: "#000000"
        opacity: 0.4
        
        // Animerad övergång
        NumberAnimation on opacity {
            from: 0.0
            to: 0.4
            duration: 200
            running: connectionDialog.visible
        }
        
        MouseArea {
            anchors.fill: parent
            onClicked: {
                connectionDialog.close()
            }
        }
    }
    
    // Centrerad dialogruta
    Rectangle {
        id: dialogContainer
        width: 400
        height: 420
        anchors.centerIn: parent
        color: theme.panel
        radius: 10
        border.width: 1
        border.color: theme.panelBorder
        
        // Animerad övergång
        NumberAnimation on scale {
            from: 0.9
            to: 1.0
            duration: 200
            running: connectionDialog.visible
            easing.type: Easing.OutQuad
        }
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15
            
            // Rubrik
            Text {
                text: "Anslut till server"
                font.pixelSize: 22
                font.bold: true
                color: theme.accent
                Layout.alignment: Qt.AlignHCenter
            }
            
            // Protokollval
            RowLayout {
                Layout.fillWidth: true
                spacing: 10
                
                Text {
                    text: "Protokoll:"
                    color: theme.text
                    font.pixelSize: 14
                    Layout.preferredWidth: 100
                }
                
                ComboBox {
                    id: protocolCombo
                    model: ["FTP", "SFTP"]
                    Layout.fillWidth: true
                    
                    background: Rectangle {
                        implicitWidth: 120
                        implicitHeight: 40
                        radius: 5
                        color: theme.listItem
                        border.width: 1
                        border.color: theme.panelBorder
                    }
                    
                    contentItem: Text {
                        text: protocolCombo.displayText
                        color: theme.text
                        font.pixelSize: 14
                        leftPadding: 10
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    popup: Popup {
                        y: protocolCombo.height
                        width: protocolCombo.width
                        implicitHeight: contentItem.implicitHeight
                        padding: 1
                        
                        contentItem: ListView {
                            clip: true
                            implicitHeight: contentHeight
                            model: protocolCombo.popup.visible ? protocolCombo.model : null
                            
                            delegate: ItemDelegate {
                                width: protocolCombo.width
                                
                                contentItem: Text {
                                    text: modelData
                                    color: theme.text
                                    font.pixelSize: 14
                                    elide: Text.ElideRight
                                    verticalAlignment: Text.AlignVCenter
                                }
                                
                                highlighted: protocolCombo.highlightedIndex === index
                                
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
            }
            
            // Servernamn
            RowLayout {
                Layout.fillWidth: true
                spacing: 10
                
                Text {
                    text: "Server:"
                    color: theme.text
                    font.pixelSize: 14
                    Layout.preferredWidth: 100
                }
                
                TextField {
                    id: serverField
                    Layout.fillWidth: true
                    placeholderText: "ex. minserver.se"
                    color: theme.text
                    
                    background: Rectangle {
                        implicitWidth: 200
                        implicitHeight: 40
                        radius: 5
                        color: theme.listItem
                        border.width: 1
                        border.color: theme.panelBorder
                    }
                    
                    // Rensa eventuella standard stylingregler
                    onFocusChanged: {
                        if (focus) {
                            color = theme.text;
                        }
                    }
                }
            }
            
            // Port
            RowLayout {
                Layout.fillWidth: true
                spacing: 10
                
                Text {
                    text: "Port:"
                    color: theme.text
                    font.pixelSize: 14
                    Layout.preferredWidth: 100
                }
                
                TextField {
                    id: portField
                    Layout.fillWidth: true
                    placeholderText: "21"
                    text: protocolCombo.currentText === "FTP" ? "21" : "22"
                    color: theme.text
                    
                    background: Rectangle {
                        implicitWidth: 200
                        implicitHeight: 40
                        radius: 5
                        color: theme.listItem
                        border.width: 1
                        border.color: theme.panelBorder
                    }
                }
            }
            
            // Användarnamn
            RowLayout {
                Layout.fillWidth: true
                spacing: 10
                
                Text {
                    text: "Användare:"
                    color: theme.text
                    font.pixelSize: 14
                    Layout.preferredWidth: 100
                }
                
                TextField {
                    id: usernameField
                    Layout.fillWidth: true
                    placeholderText: "Användarnamn"
                    color: theme.text
                    
                    background: Rectangle {
                        implicitWidth: 200
                        implicitHeight: 40
                        radius: 5
                        color: theme.listItem
                        border.width: 1
                        border.color: theme.panelBorder
                    }
                }
            }
            
            // Lösenord
            RowLayout {
                Layout.fillWidth: true
                spacing: 10
                
                Text {
                    text: "Lösenord:"
                    color: theme.text
                    font.pixelSize: 14
                    Layout.preferredWidth: 100
                }
                
                TextField {
                    id: passwordField
                    Layout.fillWidth: true
                    placeholderText: "Lösenord"
                    color: theme.text
                    echoMode: TextField.Password
                    
                    background: Rectangle {
                        implicitWidth: 200
                        implicitHeight: 40
                        radius: 5
                        color: theme.listItem
                        border.width: 1
                        border.color: theme.panelBorder
                    }
                }
            }
            
            // Standardkatalog
            RowLayout {
                Layout.fillWidth: true
                spacing: 10
                
                Text {
                    text: "Katalog:"
                    color: theme.text
                    font.pixelSize: 14
                    Layout.preferredWidth: 100
                }
                
                TextField {
                    id: directoryField
                    Layout.fillWidth: true
                    placeholderText: "/"
                    color: theme.text
                    
                    background: Rectangle {
                        implicitWidth: 200
                        implicitHeight: 40
                        radius: 5
                        color: theme.listItem
                        border.width: 1
                        border.color: theme.panelBorder
                    }
                }
            }
            
            // Alternativ för att spara anslutningen
            CheckBox {
                id: saveConnectionCheck
                text: "Spara anslutning"
                checked: true
                
                indicator: Rectangle {
                    implicitWidth: 20
                    implicitHeight: 20
                    radius: 3
                    color: theme.listItem
                    border.color: theme.panelBorder
                    border.width: 1
                    
                    Rectangle {
                        anchors.centerIn: parent
                        width: 12
                        height: 12
                        radius: 2
                        color: theme.accent
                        visible: saveConnectionCheck.checked
                    }
                }
                
                contentItem: Text {
                    text: saveConnectionCheck.text
                    color: theme.text
                    font.pixelSize: 14
                    leftPadding: saveConnectionCheck.indicator.width + 10
                    verticalAlignment: Text.AlignVCenter
                }
            }
            
            // Knappar
            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 10
                spacing: 10
                
                Item {
                    Layout.fillWidth: true
                }
                
                Button {
                    text: "Avbryt"
                    implicitWidth: 120
                    implicitHeight: 40
                    
                    background: Rectangle {
                        radius: 20
                        color: parent.down ? Qt.darker(theme.listItem, 1.3) : 
                               parent.hovered ? Qt.darker(theme.listItem, 1.1) : theme.listItem
                        border.width: 1
                        border.color: theme.panelBorder
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        color: theme.text
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: {
                        connectionDialog.close()
                    }
                }
                
                Button {
                    text: "Anslut"
                    implicitWidth: 120
                    implicitHeight: 40
                    
                    background: Rectangle {
                        radius: 20
                        color: parent.down ? Qt.darker(theme.accent, 1.3) : 
                               parent.hovered ? Qt.darker(theme.accent, 1.1) : theme.accent
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        color: "#ffffff"
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: {
                        // Skapa ett nytt anslutningsobjekt
                        var newConnection = {
                            name: serverField.text,
                            host: serverField.text,
                            port: parseInt(portField.text),
                            username: usernameField.text,
                            protocol: protocolCombo.currentText
                        };
                        
                        // Lägg till i sparade servrar om användaren vill det
                        if (saveConnectionCheck.checked) {
                            mainWindow.savedServers.push(newConnection);
                        }
                        
                        console.log("Ansluter till " + serverField.text + ":" + portField.text);
                        connectionDialog.close();
                    }
                }
            }
        }
    }
} 