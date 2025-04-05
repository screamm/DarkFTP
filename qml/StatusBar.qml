import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: statusBar
    height: 30
    
    // Tillgång till tema från applikationsfönstret
    property var theme: mainWindow.theme
    
    // Status properties
    property string statusText: "Klar"
    property string connectionStatus: "Frånkopplad"
    property int activeTransfers: 0
    property int queuedTransfers: 0
    property bool isConnected: false
    
    // Använd temafärger för bakgrund och kanter
    color: theme.panel
    border.width: 1
    border.color: theme.panelBorder
    
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 20
        
        // Statusmeddelande
        Text {
            text: statusBar.statusText
            color: theme.text
            font.pixelSize: 12
            Layout.fillWidth: true
        }
        
        // Anslutningsstatus
        Row {
            spacing: 5
            
            Rectangle {
                width: 8
                height: 8
                radius: 4
                color: statusBar.isConnected ? "#4CAF50" : "#F44336"
                anchors.verticalCenter: parent.verticalCenter
            }
            
            Text {
                text: statusBar.connectionStatus
                color: theme.text
                font.pixelSize: 12
                anchors.verticalCenter: parent.verticalCenter
            }
        }
        
        // Överföringsindikator
        Row {
            spacing: 5
            visible: statusBar.activeTransfers > 0 || statusBar.queuedTransfers > 0
            
            Image {
                source: "../icons/transfer.png"
                width: 16
                height: 16
                anchors.verticalCenter: parent.verticalCenter
                
                // Fallback om bilden inte hittas
                Rectangle {
                    visible: parent.status !== Image.Ready
                    anchors.fill: parent
                    color: "transparent"
                    border.color: theme.accent
                    border.width: 1
                    
                    Text {
                        anchors.centerIn: parent
                        text: "T"
                        color: theme.accent
                        font.pixelSize: 10
                        font.bold: true
                    }
                }
            }
            
            Text {
                text: statusBar.activeTransfers + " aktiva, " + statusBar.queuedTransfers + " i kö"
                color: theme.text
                font.pixelSize: 12
                anchors.verticalCenter: parent.verticalCenter
            }
        }
        
        // Tema-väljare (kan flyttas till inställningsmenyn senare)
        ComboBox {
            id: themeSelector
            model: Object.keys(mainWindow.themes)
            currentIndex: model.indexOf(mainWindow.currentThemeName)
            Layout.preferredWidth: 120
            
            background: Rectangle {
                implicitWidth: 120
                implicitHeight: 25
                radius: 3
                color: theme.listItem
                border.width: 1
                border.color: theme.panelBorder
            }
            
            contentItem: Text {
                text: themeSelector.displayText
                color: theme.text
                font.pixelSize: 12
                leftPadding: 5
                verticalAlignment: Text.AlignVCenter
            }
            
            popup: Popup {
                y: themeSelector.height
                width: themeSelector.width
                implicitHeight: contentItem.implicitHeight
                padding: 1
                
                contentItem: ListView {
                    clip: true
                    implicitHeight: contentHeight
                    model: themeSelector.popup.visible ? themeSelector.model : null
                    
                    delegate: ItemDelegate {
                        width: themeSelector.width
                        
                        contentItem: Text {
                            text: modelData
                            color: theme.text
                            font.pixelSize: 12
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                        }
                        
                        highlighted: themeSelector.highlightedIndex === index
                        
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
                    radius: 3
                }
            }
            
            onActivated: {
                mainWindow.currentThemeName = model[index];
            }
        }
    }
} 