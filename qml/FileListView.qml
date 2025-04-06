import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
// import QtQuick.Transitions 2.15 // Tillfälligt bortkommenterad
import DarkFTP 1.0

Rectangle {
    id: fileListRoot
    color: "transparent"
    
    property var theme: mainWindow.theme
    property var model
    
    signal fileDoubleClicked(int index)
    signal fileDragged(int index)
    signal fileDropped(string sourcePath, string targetPath)
    
    Rectangle {
        id: headerRect
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 30
        color: theme.header
        radius: 3
        
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            spacing: 5
            
            Text { text: "Namn"; font.pixelSize: 14; font.bold: true; color: theme.accent; Layout.preferredWidth: parent.width * 0.4 }
            Text { text: "Storlek"; font.pixelSize: 14; font.bold: true; color: theme.accent; Layout.preferredWidth: parent.width * 0.2 }
            Text { text: "Datum"; font.pixelSize: 14; font.bold: true; color: theme.accent; Layout.fillWidth: true }
        }
    }
    
    ListView {
        id: fileListView
        anchors.top: headerRect.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.topMargin: 1
        clip: true
        model: fileListRoot.model
        
        ScrollBar.vertical: ScrollBar { active: true }
        
        delegate: Rectangle {
            id: delegateItem
            width: fileListView.width
            height: 30
            color: index % 2 === 0 ? theme.listItem : theme.listItemAlt
            
            Rectangle {
                id: hoverRect
                anchors.fill: parent
                color: theme.highlight
                opacity: 0
            }
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                spacing: 5
                
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
                            color: model.isDirectory ? theme.accent : "transparent"
                            opacity: model.isDirectory ? 0.3 : 0
                            radius: 2
                            
                            Text {
                                anchors.centerIn: parent
                                text: model.isDirectory ? "D" : "F"
                                color: theme.text
                                font.pixelSize: 10
                            }
                        }
                    }
                    
                    // Filnamn
                    Text {
                        text: model.fileName
                        font.pixelSize: 14
                        color: theme.text
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                }
                
                // Filstorlek
                Text {
                    text: model.fileSize
                    font.pixelSize: 14
                    color: theme.text
                    Layout.preferredWidth: parent.width * 0.2
                }
                
                // Datum
                Text {
                    text: model.fileDate
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
                
                onEntered: function() {
                    hoverRect.opacity = 0.2
                }
                
                onExited: function() {
                    hoverRect.opacity = 0
                }
                
                onClicked: function(mouse) {
                    fileListView.currentIndex = index
                    if (mouse.button === Qt.RightButton) contextMenu.popup()
                }
                
                onDoubleClicked: function() {
                    fileListRoot.fileDoubleClicked(index)
                }
                
                // Stöd för drag and drop
                drag.target: dragRect
                drag.axis: Drag.XAndYAxis
                
                onPressed: function(mouse) {
                    if (mouse.button === Qt.LeftButton) {
                        dragRect.dragFilePath = model.filePath
                        dragRect.text = model.fileName
                        dragRect.x = mouseArea.mouseX - dragRect.Drag.hotSpot.x
                        dragRect.y = mouseArea.mouseY - dragRect.Drag.hotSpot.y
                    }
                }
                
                onReleased: function() {
                    if (drag.active) {
                        dragRect.Drag.drop()
                    }
                    dragRect.visible = false
                    dragRect.dragFilePath = ""
                }
                
                onPositionChanged: function(mouse) {
                    if (drag.active && !dragRect.visible) {
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
                z: 1
                
                property string dragFilePath: ""
                property string text: ""
                
                Text {
                    anchors.centerIn: parent
                    text: dragRect.text
                    color: theme.accent
                    font.pixelSize: 14
                }
                
                Drag.hotSpot.x: 10
                Drag.hotSpot.y: 10
                
                states: State {
                    name: "DraggingState"
                    ParentChange {
                        target: dragRect
                        parent: fileListView
                        x: fileListView.mapFromItem(delegateItem.mouseArea, delegateItem.mouseArea.mouseX - Drag.hotSpot.x).x
                        y: fileListView.mapFromItem(delegateItem.mouseArea, delegateItem.mouseArea.mouseY - Drag.hotSpot.y).y
                    }
                }
            }
            
            // Kontextmeny
            Menu {
                id: contextMenu
                
                MenuItem {
                    text: model.isDirectory ? "Öppna mapp" : "Öppna fil"
                    onTriggered: fileListRoot.fileDoubleClicked(index)
                    enabled: model.filePath
                }
                
                MenuItem {
                    text: "Ladda ner/upp"
                    onTriggered: console.log("Ladda ner/upp: " + model.fileName)
                    enabled: model.filePath
                }
                
                MenuItem {
                    text: "Döp om"
                    onTriggered: {
                        console.log("Döp om (behöver dialog): " + model.filePath)
                    }
                    enabled: model.filePath
                }
                
                MenuItem {
                    text: "Ta bort"
                    onTriggered: {
                        console.log("Försöker ta bort: " + model.filePath)
                        fileListRoot.model.deletePath(model.filePath)
                    }
                    enabled: model.filePath
                }
                
                MenuSeparator {}
                
                MenuItem {
                    text: "Skapa mapp här"
                    onTriggered: {
                        console.log("Skapa mapp här (behöver dialog) i: " + fileListRoot.model.currentPath)
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
            id: dropArea
            anchors.fill: parent
            onDropped: (drop) => {
                if (drop.source === dragRect && dragRect.dragFilePath !== "") {
                    var sourcePath = dragRect.dragFilePath
                    var targetPath = fileListRoot.model.currentPath
                    console.log("Fil släppt internt:", sourcePath, "till", targetPath)
                    fileListRoot.fileDropped(sourcePath, targetPath)
                } else if (drop.hasUrls) {
                    console.log("Externa filer släppta:", drop.urls)
                    for (var i = 0; i < drop.urls.length; ++i) {
                        var externalSourcePath = drop.urls[i].toString()
                        if (externalSourcePath.startsWith("file:///")) {
                            externalSourcePath = externalSourcePath.substring(8)
                            if (Qt.platform.os === "windows" && externalSourcePath.length > 1 && externalSourcePath[1] === ':') {
                            } else if (Qt.platform.os === "windows") {
                                externalSourcePath = "/" + externalSourcePath
                            }
                        }
                        console.log("  -> Behandlar:", externalSourcePath)
                        fileListRoot.fileDropped(externalSourcePath, fileListRoot.model.currentPath)
                    }
                } else {
                    console.warn("Drop utan förväntad data. Källa:", drop.source)
                }
            }
        }
    }
} 