import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: fileListRoot
    color: "transparent"
    
    // Tillgång till tema från applikationsfönstret
    property var theme: mainWindow.theme
    
    // Egenskaper och signaler
    property var model
    
    // Signal för att öppna en fil/mapp
    signal fileDoubleClicked(int index)
    signal fileDragged(int index)
    signal fileDropped(string sourceUrl, string targetUrl)
    
    // Lista med header
    Rectangle {
        id: headerRect
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 30
        color: theme.header
        radius: 3
        
        // Filinformation i sidhuvudet
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            spacing: 5
            
            Text {
                text: "Namn"
                font.pixelSize: 14
                font.bold: true
                color: theme.accent
                Layout.preferredWidth: parent.width * 0.4
            }
            
            Text {
                text: "Storlek"
                font.pixelSize: 14
                font.bold: true
                color: theme.accent
                Layout.preferredWidth: parent.width * 0.2
            }
            
            Text {
                text: "Datum"
                font.pixelSize: 14
                font.bold: true
                color: theme.accent
                Layout.fillWidth: true
            }
        }
    }
    
    // Fillista
    ListView {
        id: fileListView
        anchors.top: headerRect.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.topMargin: 1
        clip: true
        model: fileListRoot.model
        
        // Vertikalt rullningsfält
        ScrollBar.vertical: ScrollBar {
            active: true
        }
        
        // Delegat för filitems
        delegate: Rectangle {
            id: delegateItem
            width: fileListView.width
            height: 30
            // Växla mellan två färger för bättre läsbarhet
            color: index % 2 === 0 ? theme.listItem : theme.listItemAlt
            
            // Hover-effekt
            Rectangle {
                id: hoverRect
                anchors.fill: parent
                color: theme.highlight
                opacity: 0
            }
            
            // Filinformation för varje rad
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                spacing: 5
                
                // Filnamn med ikon
                RowLayout {
                    spacing: 5
                    Layout.preferredWidth: parent.width * 0.4
                    
                    // Filtypsikon
                    Rectangle {
                        width: 16
                        height: 16
                        color: "transparent"
                        
                        Rectangle {
                            anchors.fill: parent
                            color: isDirectory ? theme.accent : "transparent"
                            opacity: isDirectory ? 0.3 : 0
                            radius: 2
                            
                            Text {
                                anchors.centerIn: parent
                                text: isDirectory ? "D" : "F"
                                color: theme.text
                                font.pixelSize: 10
                            }
                        }
                    }
                    
                    // Filnamn
                    Text {
                        text: fileName
                        font.pixelSize: 14
                        color: theme.text
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                }
                
                // Filstorlek
                Text {
                    text: fileSize
                    font.pixelSize: 14
                    color: theme.text
                    Layout.preferredWidth: parent.width * 0.2
                }
                
                // Datum
                Text {
                    text: fileDate
                    font.pixelSize: 14
                    color: theme.text
                    Layout.fillWidth: true
                }
            }
            
            // Ändra radutseende vid hovring
            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                
                onEntered: {
                    hoverRect.opacity = 0.2
                }
                
                onExited: {
                    hoverRect.opacity = 0
                }
                
                onClicked: {
                    fileListView.currentIndex = index
                    
                    // Visa kontextmeny vid högerklick
                    if (mouse.button === Qt.RightButton) {
                        contextMenu.popup()
                    }
                }
                
                onDoubleClicked: {
                    fileListRoot.fileDoubleClicked(index)
                }
                
                // Stöd för drag and drop
                drag.target: dragRect
                drag.axis: Drag.XAndYAxis
                
                onPressed: {
                    if (mouse.button === Qt.LeftButton) {
                        dragRect.x = mouseArea.mouseX
                        dragRect.y = mouseArea.mouseY
                    }
                }
                
                onReleased: {
                    dragRect.Drag.drop()
                    dragRect.x = 0
                    dragRect.y = 0
                    dragRect.visible = false
                }
                
                // Dra filen om den dras mer än 20 pixlar
                onPositionChanged: {
                    if (drag.active) {
                        dragRect.text = fileName
                        dragRect.visible = true
                        fileListRoot.fileDragged(index)
                    }
                }
            }
            
            // Visuell drag-representation
            Rectangle {
                id: dragRect
                visible: false
                width: 180
                height: 40
                radius: 5
                color: theme.panel
                border.width: 1
                border.color: theme.accent
                opacity: 0.9
                
                // För att dra och släppa
                Drag.active: mouseArea.drag.active
                Drag.hotSpot.x: width / 2
                Drag.hotSpot.y: height / 2
                
                property string text: ""
                
                Text {
                    anchors.centerIn: parent
                    text: dragRect.text
                    color: theme.accent
                    font.pixelSize: 14
                }
                
                states: State {
                    when: mouseArea.drag.active
                    
                    ParentChange {
                        target: dragRect
                        parent: fileListRoot
                    }
                    
                    AnchorChanges {
                        target: dragRect
                        anchors.top: undefined
                        anchors.left: undefined
                    }
                }
            }
            
            // Kontextmeny
            Menu {
                id: contextMenu
                
                MenuItem {
                    text: isDirectory ? "Öppna mapp" : "Öppna fil"
                    onTriggered: {
                        fileListRoot.fileDoubleClicked(index)
                    }
                }
                
                MenuItem {
                    text: "Ladda ner/upp"
                    onTriggered: {
                        console.log("Laddar ner/upp: " + fileName)
                    }
                }
                
                MenuItem {
                    text: "Döp om"
                    onTriggered: {
                        console.log("Döp om: " + fileName)
                    }
                }
                
                MenuItem {
                    text: "Ta bort"
                    onTriggered: {
                        console.log("Ta bort: " + fileName)
                    }
                }
            }
        }
        
        // Markera aktuellt val
        highlight: Rectangle {
            color: theme.highlight
            opacity: 0.5
            radius: 2
        }
        
        // Stöd för drop
        DropArea {
            anchors.fill: parent
            onDropped: {
                console.log("Fil släppt:", drop.source.text)
                fileListRoot.fileDropped(drop.source.text, "")
            }
        }
    }
} 