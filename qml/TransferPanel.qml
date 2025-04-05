import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: transferPanelRoot
    color: "transparent"
    
    // Tillgång till tema från applikationsfönstret
    property var theme: mainWindow.theme
    
    // Exempelmodell för överföringar
    ListModel {
        id: transferModel
        
        ListElement {
            fileName: "dokument.pdf"
            sourceFile: "/home/user/dokument.pdf"
            targetFile: "C:/Users/david/Downloads/dokument.pdf"
            fileSize: "2.5 MB"
            transferProgress: 0.75
            transferSpeed: "1.2 MB/s"
            timeRemaining: "5s"
            isUpload: false
            status: "Pågår"
        }
        
        ListElement {
            fileName: "bilder.zip"
            sourceFile: "C:/Users/david/Documents/bilder.zip"
            targetFile: "/home/user/uploads/bilder.zip"
            fileSize: "15.8 MB"
            transferProgress: 0.35
            transferSpeed: "850 KB/s"
            timeRemaining: "25s"
            isUpload: true
            status: "Pågår"
        }
        
        ListElement {
            fileName: "config.json"
            sourceFile: "/home/user/config.json"
            targetFile: "C:/Users/david/Downloads/config.json"
            fileSize: "12 KB"
            transferProgress: 1.0
            transferSpeed: "0 KB/s"
            timeRemaining: "0s"
            isUpload: false
            status: "Klar"
        }
        
        ListElement {
            fileName: "backup.sql"
            sourceFile: "C:/Users/david/Documents/backup.sql"
            targetFile: "/home/user/uploads/backup.sql"
            fileSize: "4.3 MB"
            transferProgress: 0.0
            transferSpeed: "0 KB/s"
            timeRemaining: "Väntar"
            isUpload: true
            status: "I kö"
        }
    }
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        
        Rectangle {
            id: headerRect
            Layout.fillWidth: true
            height: 30
            color: theme.header
            radius: 3
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                
                Text {
                    text: "Överföringar"
                    font.pixelSize: 14
                    font.bold: true
                    color: theme.accent
                    Layout.fillWidth: true
                }
                
                Text {
                    text: transferModel.count + " filer"
                    font.pixelSize: 12
                    color: theme.text
                }
            }
        }
        
        ListView {
            id: transferListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: transferModel
            
            ScrollBar.vertical: ScrollBar {
                active: true
            }
            
            delegate: Rectangle {
                id: transferItem
                width: ListView.view.width
                height: 60
                color: index % 2 === 0 ? theme.listItem : theme.listItemAlt
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 4
                    
                    // Övre rad med filnamn och status
                    RowLayout {
                        Layout.fillWidth: true
                        
                        // Riktningspil
                        Rectangle {
                            width: 16
                            height: 16
                            color: "transparent"
                            
                            Text {
                                anchors.centerIn: parent
                                text: isUpload ? "↑" : "↓"
                                color: isUpload ? "#4CAF50" : "#2196F3"
                                font.pixelSize: 14
                                font.bold: true
                            }
                        }
                        
                        // Filnamn
                        Text {
                            text: fileName
                            font.pixelSize: 14
                            font.bold: true
                            color: theme.text
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        
                        // Filstorlek
                        Text {
                            text: fileSize
                            font.pixelSize: 12
                            color: theme.text
                            horizontalAlignment: Text.AlignRight
                        }
                        
                        // Status
                        Rectangle {
                            width: statusText.width + 10
                            height: statusText.height + 4
                            radius: height / 2
                            color: status === "Klar" ? "#4CAF50" : 
                                  status === "Pågår" ? "#2196F3" : 
                                  status === "I kö" ? "#FFC107" : "#F44336"
                            opacity: 0.7
                            
                            Text {
                                id: statusText
                                anchors.centerIn: parent
                                text: status
                                font.pixelSize: 10
                                color: "white"
                            }
                        }
                    }
                    
                    // Mellanrad med källsökväg och målsökväg
                    Text {
                        text: isUpload ? 
                              sourceFile + " → " + targetFile : 
                              sourceFile + " → " + targetFile
                        font.pixelSize: 10
                        color: theme.text
                        opacity: 0.7
                        elide: Text.ElideMiddle
                        Layout.fillWidth: true
                    }
                    
                    // Nedre rad med framstegsindikator
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        
                        // Framstegsindikator
                        ProgressBar {
                            from: 0
                            to: 1
                            value: transferProgress
                            Layout.fillWidth: true
                            
                            background: Rectangle {
                                implicitWidth: 200
                                implicitHeight: 6
                                color: theme.panel
                                radius: 3
                            }
                            
                            contentItem: Item {
                                implicitWidth: 200
                                implicitHeight: 6
                                
                                Rectangle {
                                    width: transferProgress * parent.width
                                    height: parent.height
                                    radius: 3
                                    color: isUpload ? "#4CAF50" : "#2196F3"
                                    
                                    // Pulserings-effekt för aktiva överföringar
                                    Rectangle {
                                        anchors.right: parent.right
                                        anchors.verticalCenter: parent.verticalCenter
                                        width: 12
                                        height: 12
                                        radius: 6
                                        color: parent.color
                                        visible: status === "Pågår"
                                        opacity: status === "Pågår" ? 0.7 : 0
                                        
                                        SequentialAnimation on opacity {
                                            loops: Animation.Infinite
                                            running: status === "Pågår"
                                            NumberAnimation {
                                                from: 0.7
                                                to: 0.3
                                                duration: 1000
                                                easing.type: Easing.InOutQuad
                                            }
                                            NumberAnimation {
                                                from: 0.3
                                                to: 0.7
                                                duration: 1000
                                                easing.type: Easing.InOutQuad
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        
                        // Överföringshastighet och återstående tid
                        Text {
                            text: transferSpeed + " · " + timeRemaining
                            font.pixelSize: 10
                            color: theme.text
                            visible: status === "Pågår"
                        }
                    }
                }
                
                // Knapprad för att pausa/fortsätta/avbryta överföring
                Row {
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 4
                    spacing: 3
                    visible: status === "Pågår" || status === "I kö"
                    
                    // Pausa/Fortsätta-knapp
                    Rectangle {
                        width: 18
                        height: 18
                        radius: 3
                        color: "transparent"
                        
                        Text {
                            anchors.centerIn: parent
                            text: status === "Pågår" ? "⏸" : "▶"
                            color: theme.text
                            font.pixelSize: 12
                        }
                        
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                console.log(status === "Pågår" ? "Pausar överföring: " : "Fortsätter överföring: " + fileName)
                                // Här kan vi implementera paus/fortsätt-logik
                            }
                        }
                    }
                    
                    // Avbryt-knapp
                    Rectangle {
                        width: 18
                        height: 18
                        radius: 3
                        color: "transparent"
                        
                        Text {
                            anchors.centerIn: parent
                            text: "✕"
                            color: theme.text
                            font.pixelSize: 12
                        }
                        
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                console.log("Avbryter överföring: " + fileName)
                                // Här kan vi implementera avbryt-logik
                            }
                        }
                    }
                }
            }
        }
        
        // Knapprad längst ner
        Rectangle {
            Layout.fillWidth: true
            height: 40
            color: theme.panel
            radius: 3
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                
                // Rensa slutförda-knapp
                Button {
                    text: "Rensa slutförda"
                    
                    background: Rectangle {
                        implicitWidth: 120
                        implicitHeight: 30
                        radius: 15
                        color: parent.down ? Qt.darker(theme.listItem, 1.3) : 
                               parent.hovered ? Qt.darker(theme.listItem, 1.1) : theme.listItem
                        border.width: 1
                        border.color: theme.panelBorder
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        color: theme.text
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: {
                        console.log("Rensar slutförda överföringar")
                        // Här kan vi ta bort slutförda överföringar från modellen
                    }
                }
                
                Item {
                    Layout.fillWidth: true
                }
                
                // Avbryt alla-knapp
                Button {
                    text: "Avbryt alla"
                    
                    background: Rectangle {
                        implicitWidth: 100
                        implicitHeight: 30
                        radius: 15
                        color: parent.down ? Qt.darker(theme.listItem, 1.3) : 
                               parent.hovered ? Qt.darker(theme.listItem, 1.1) : theme.listItem
                        border.width: 1
                        border.color: theme.panelBorder
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        color: theme.text
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: {
                        console.log("Avbryter alla överföringar")
                        // Här kan vi avbryta alla pågående överföringar
                    }
                }
            }
        }
    }
} 