import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "." as DarkFTP

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 1200
    height: 800
    title: "DarkFTP"
    color: "transparent"
    
    // Gör fönstret tillgängligt globalt
    property var applicationWindow: mainWindow
    
    // Tema-hantering
    property var themes: {
        "Terminal": {
            background: "#0C1021",
            gradient1: "#0C1021",
            gradient2: "#142438",
            panel: "#0F1A2A",
            accent: "#32CD32",  // Saftigt grönt
            text: "#D0D0D0",
            header: "#152A45",
            listItem: "#121C2C",
            listItemAlt: "#0E1825",
            panelBorder: "#1E3C64",
            highlight: "#2A4E80"
        },
        "Mörkblå": {
            background: "#0A192F",
            gradient1: "#0A192F",
            gradient2: "#0F2A4A",
            panel: "#0F1E38",
            accent: "#64FFDA",  // Turkos
            text: "#E6F1FF",
            header: "#172B4D",
            listItem: "#112240",
            listItemAlt: "#0D1B31",
            panelBorder: "#233554",
            highlight: "#2D4A73"
        },
        "Mörklila": {
            background: "#13111C",
            gradient1: "#13111C",
            gradient2: "#221B3A",
            panel: "#1C1729",
            accent: "#BD93F9",  // Lila
            text: "#F8F8F2",
            header: "#272038",
            listItem: "#1E1635",
            listItemAlt: "#191127",
            panelBorder: "#332A4B",
            highlight: "#493D6B"
        },
        "Mörk": {
            background: "#1A1A1A",
            gradient1: "#1A1A1A",
            gradient2: "#2C2C2C",
            panel: "#242424",
            accent: "#61AFEF",  // Ljusblå
            text: "#ABB2BF",
            header: "#333333",
            listItem: "#2A2A2A",
            listItemAlt: "#242424",
            panelBorder: "#3E3E3E",
            highlight: "#3A3A3A"
        },
        "Grönt": {
            background: "#0B2B26",
            gradient1: "#0B2B26",
            gradient2: "#144D40",
            panel: "#0E352F",
            accent: "#00FF9C",  // Neongrönt
            text: "#D8F3EC",
            header: "#13463E",
            listItem: "#103C35",
            listItemAlt: "#0C2E29",
            panelBorder: "#1E5A4F",
            highlight: "#2B7267"
        },
        "Cyber": {
            background: "#0D0221",
            gradient1: "#0D0221",
            gradient2: "#3B0069",
            panel: "#14042C",
            accent: "#00F3FF",  // Neonblå
            text: "#FFFFFF",
            header: "#210650",
            listItem: "#190440",
            listItemAlt: "#110335",
            panelBorder: "#36096C",
            highlight: "#4C0D99"
        }
    }
    
    // Aktuellt tema
    property string currentThemeName: "Terminal"
    property var theme: themes[currentThemeName]
    
    // Sparade servrar (Exempeldata)
    property var savedServers: [
        { name: "Exempelserver", host: "ftp.example.com", port: 21, username: "user", protocol: "FTP" },
        { name: "SFTP Server", host: "sftp.example.org", port: 22, username: "sshuser", protocol: "SFTP" }
    ]
    
    // Huvudbakgrund med övertoningsfärg baserad på tema
    background: Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: theme.gradient1 }
            GradientStop { position: 1.0; color: theme.gradient2 }
        }
    }
    
    // Komponenter för dialogrutor
    DarkFTP.ConnectionDialog {
        id: connectionDialog
        anchors.centerIn: parent
    }
    
    // Huvudlayout
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        
        // Navigationsfält
        DarkFTP.NavigationBar {
            id: navBar
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            
            onConnectRequested: {
                connectionDialog.open();
            }
        }
        
        // Huvudinnehållsområde med två fillistor och överföringsområde
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"
            
            SplitView {
                anchors.fill: parent
                orientation: Qt.Horizontal
                
                // Lokal fillista
                Rectangle {
                    SplitView.preferredWidth: parent.width * 0.4
                    SplitView.minimumWidth: 300
                    color: "transparent"
                    
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 5
                        
                        Text {
                            text: "Lokal"
                            color: theme.accent
                            font.pixelSize: 16
                            font.bold: true
                        }
                        
                        Rectangle {
                            Layout.fillWidth: true
                            height: 30
                            color: theme.panel
                            radius: 3
                            border.width: 1
                            border.color: theme.panelBorder
                            
                            TextInput {
                                id: localPathInput // Behåller ID för enkelhets skull
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                anchors.rightMargin: 10
                                verticalAlignment: Text.AlignVCenter
                                color: theme.text
                                text: "C:/Användare/david/Dokument" // Hårdkodad sökväg
                                selectByMouse: true
                                
                                onAccepted: {
                                    console.log("Byter lokal katalog till: " + text);
                                    // Ingen koppling till C++-modell
                                }
                            }
                        }
                        
                        DarkFTP.FileListView {
                            id: localFileList
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            // Borttaget: model: localFileModel
                            // Borttaget: isRemote: false
                            
                            // Återställd exempelmodell
                            model: ListModel {
                                ListElement { fileName: "Dokument"; fileSize: "Mapp"; fileDate: "2023-05-10 14:30"; isDirectory: true }
                                ListElement { fileName: "Bilder"; fileSize: "Mapp"; fileDate: "2023-05-12 09:45"; isDirectory: true }
                                ListElement { fileName: "projekt.zip"; fileSize: "15.2 MB"; fileDate: "2023-05-15 16:20"; isDirectory: false }
                                ListElement { fileName: "rapport.docx"; fileSize: "1.8 MB"; fileDate: "2023-05-16 10:15"; isDirectory: false }
                                ListElement { fileName: "presentation.pptx"; fileSize: "5.4 MB"; fileDate: "2023-05-18 13:40"; isDirectory: false }
                            }
                            
                            // Återställda signaler utan C++-modellinformation
                            onFileDoubleClicked: function(index) {
                                var item = model.get(index);
                                console.log("Local file clicked:", item.fileName, item.isDirectory ? "(directory)" : "(file)");
                                if (item.isDirectory) {
                                    console.log("Öppnar mapp: " + item.fileName); // Logik för att öppna mapp här
                                }
                            }
                            
                            onFileDragged: function(index) {
                                console.log("Local file dragged:", model.get(index).fileName);
                            }
                            
                            onFileDropped: function(sourceUrl, targetUrl) {
                                console.log("File dropped to local:", sourceUrl, "->", targetUrl);
                                // Implementera filöverföring här
                            }
                        }
                    }
                }
                
                // Överföringsområde
                Rectangle {
                    SplitView.preferredWidth: parent.width * 0.2
                    SplitView.minimumWidth: 200
                    color: "transparent"
                    
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 5
                        
                        Text {
                            text: "Överföringar"
                            color: theme.accent
                            font.pixelSize: 16
                            font.bold: true
                        }
                        
                        DarkFTP.TransferPanel {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                        }
                    }
                }
                
                // Fjärrfillista
                Rectangle {
                    SplitView.preferredWidth: parent.width * 0.4
                    SplitView.minimumWidth: 300
                    color: "transparent"
                    
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 5
                        
                        Text {
                            text: "Server"
                            color: theme.accent
                            font.pixelSize: 16
                            font.bold: true
                        }
                        
                        Rectangle {
                            Layout.fillWidth: true
                            height: 30
                            color: theme.panel
                            radius: 3
                            border.width: 1
                            border.color: theme.panelBorder
                            
                            TextInput {
                                id: remotePathInput // Behåller ID
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                anchors.rightMargin: 10
                                verticalAlignment: Text.AlignVCenter
                                color: theme.text
                                text: "/home/user" // Hårdkodad sökväg
                                selectByMouse: true
                                
                                onAccepted: {
                                    console.log("Byter fjärrkatalog till: " + text);
                                    // Ingen koppling till C++-modell
                                }
                            }
                        }
                        
                        DarkFTP.FileListView {
                            id: remoteFileList
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            // Borttaget: model: remoteFileModel
                            // Borttaget: isRemote: true
                            
                            // Återställd exempelmodell
                            model: ListModel {
                                ListElement { fileName: "public_html"; fileSize: "Mapp"; fileDate: "2023-05-10 14:30"; isDirectory: true }
                                ListElement { fileName: "logs"; fileSize: "Mapp"; fileDate: "2023-05-12 09:45"; isDirectory: true }
                                ListElement { fileName: "backup.zip"; fileSize: "45.7 MB"; fileDate: "2023-05-15 16:20"; isDirectory: false }
                                ListElement { fileName: "config.json"; fileSize: "8.2 KB"; fileDate: "2023-05-16 10:15"; isDirectory: false }
                                ListElement { fileName: "index.html"; fileSize: "12.4 KB"; fileDate: "2023-05-18 13:40"; isDirectory: false }
                            }
                            
                            // Återställda signaler
                            onFileDoubleClicked: function(index) {
                                var item = model.get(index);
                                console.log("Remote file clicked:", item.fileName, item.isDirectory ? "(directory)" : "(file)");
                                if (item.isDirectory) {
                                    console.log("Öppnar fjärrmapp: " + item.fileName);
                                }
                            }
                            
                            onFileDragged: function(index) {
                                console.log("Remote file dragged:", model.get(index).fileName);
                            }
                            
                            onFileDropped: function(sourceUrl, targetUrl) {
                                console.log("File dropped to remote:", sourceUrl, "->", targetUrl);
                                // Implementera filöverföring här
                            }
                        }
                    }
                }
            }
        }
        
        // Statusfält
        DarkFTP.StatusBar {
            Layout.fillWidth: true
        }
    }
} 