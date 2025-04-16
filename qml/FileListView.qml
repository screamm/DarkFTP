import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
// import QtQuick.Transitions 2.15 // Tillfälligt bortkommenterad
import DarkFTP 1.0

Rectangle {
    id: fileListRoot
    color: "transparent"
    
    // Använd ThemeManager för tema istället för property binding
    readonly property var theme: ThemeManager.theme
    property var model
    
    signal fileDoubleClicked(int index)
    signal fileDragged(int index)
    signal fileDropped(string sourcePath, string targetPath)
    
    // Cache för filtypsikoner för att undvika omkompilering av bildresurser
    property var iconCache: ({})
    
    Rectangle {
        id: headerRect
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 30
        color: theme.header
        radius: 3
        
        // Förinställda värden för att undvika bindningar
        readonly property real nameWidth: 0.4
        readonly property real sizeWidth: 0.2
        
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
                Layout.preferredWidth: parent.width * headerRect.nameWidth
            }
            
            Text {
                text: "Storlek"
                font.pixelSize: 14
                font.bold: true
                color: theme.accent
                Layout.preferredWidth: parent.width * headerRect.sizeWidth
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
    
    ListView {
        id: fileListView
        anchors.top: headerRect.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.topMargin: 1
        clip: true
        model: fileListRoot.model
        
        // Optimera genom att förladda bara de objekt som syns
        cacheBuffer: 200
        
        ScrollBar.vertical: ScrollBar { 
            active: true
            // Optimera genom att skapa transparent bakgrund direkt
            // istället för att använda komplexa bindningar
            background: Rectangle {
                color: "transparent"
            }
        }
        
        delegate: Item {
            id: delegateRoot
            width: fileListView.width
            height: 30
            
            // Undvik att skapa nya objekt vid varje omrenderad rad
            Component.onCompleted: {
                // Använd index modulo 2 för att avgöra bakgrundsfärgen
                delegateBackground.color = (index % 2 === 0) ? 
                    theme.listItem : theme.listItemAlt;
            }
            
            Rectangle {
                id: delegateBackground
                anchors.fill: parent
                // Färgsättning sker i Component.onCompleted för att undvika bindningar
            }
            
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
                    Layout.preferredWidth: parent.width * headerRect.nameWidth
                    
                    // Filtypsikon - optimerad genom att använda förenklad kod
                    Rectangle {
                        width: 16
                        height: 16
                        // Använd färdig färgsättning istället för bindning
                        color: model.isDirectory ? Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.3) : "transparent"
                        radius: 2
                        visible: true
                        
                        Text {
                            anchors.centerIn: parent
                            text: model.isDirectory ? "D" : "F"
                            color: theme.text
                            font.pixelSize: 10
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
                    Layout.preferredWidth: parent.width * headerRect.sizeWidth
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
                
                onEntered: hoverRect.opacity = 0.2
                onExited: hoverRect.opacity = 0
                
                onClicked: {
                    fileListView.currentIndex = index
                    if (mouse.button === Qt.RightButton) contextMenuLoader.showMenu()
                }
                
                onDoubleClicked: fileListRoot.fileDoubleClicked(index)
                
                // Stöd för drag and drop
                drag.target: dragRect
                drag.axis: Drag.XAndYAxis
                
                // Använd direkta funktioner för onPressed/Released för bättre prestanda
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
            
            // Visuell drag-representation - optimerad genom att reducera bindningar
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
                    // Enklare ParentChange som förändrar mindre egenskaper
                    ParentChange {
                        target: dragRect
                        parent: fileListView
                    }
                }
            }
            
            // Kontextmeny - använd lazy loading med Loader för att minska uppstartstiden
            Loader {
                id: contextMenuLoader
                active: false
                
                sourceComponent: Menu {
                    id: contextMenu
                    
                    MenuItem {
                        text: model.isDirectory ? "Öppna mapp" : "Öppna fil"
                        onTriggered: fileListRoot.fileDoubleClicked(index)
                        enabled: model.filePath !== ""
                    }
                    
                    MenuItem {
                        text: "Ladda ner/upp"
                        onTriggered: console.log("Ladda ner/upp: " + model.fileName)
                        enabled: model.filePath !== ""
                    }
                    
                    MenuItem {
                        text: "Döp om"
                        onTriggered: {
                            console.log("Döp om (behöver dialog): " + model.filePath)
                        }
                        enabled: model.filePath !== ""
                    }
                    
                    MenuItem {
                        text: "Ta bort"
                        onTriggered: {
                            console.log("Försöker ta bort: " + model.filePath)
                            fileListRoot.model.deletePath(model.filePath)
                        }
                        enabled: model.filePath !== ""
                    }
                    
                    MenuSeparator {}
                    
                    MenuItem {
                        text: "Skapa mapp här"
                        onTriggered: {
                            console.log("Skapa mapp här (behöver dialog) i: " + fileListRoot.model.currentPath)
                        }
                    }
                    
                    Component.onCompleted: {
                        popup();
                    }
                }
                
                function showMenu() {
                    active = true;
                }
            }
            
            // Optimera genom att ladda kontextmenyn endast när det behövs
            Connections {
                target: mouseArea
                function onClicked(mouse) {
                    if (mouse.button === Qt.RightButton) {
                        contextMenuLoader.showMenu();
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