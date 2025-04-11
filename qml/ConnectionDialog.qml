import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 6.5
import Qt.labs.platform as Platform

Popup {
    id: connectionDialog
    width: 550
    height: 550
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    
    // Tillgång till tema från applikationsfönstret, med fallback-värden
    property var theme: {
        "panel": "#2d2d30",
        "panelBorder": "#3f3f46",
        "text": "#f0f0f0",
        "accent": "#4d79ff",
        "error": "#ff6347",
        "listItem": "#252526"
    }
    
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
    
    // Egenskaper för felhantering
    property string errorText: ""
    property bool serverError: false
    property bool portError: false
    property bool keyPathError: false
    
    // Håll reda på autentiseringsmetod
    property bool usePassword: true
    property bool useKey: protocolCombo.currentText === "SFTP" && authTypeCombo.currentIndex > 0
    
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
            
            // Felmeddelandetext (visas vid fel)
            Text {
                id: errorLabel
                text: connectionDialog.errorText
                color: theme.error || "#ff6347" // Använd temats felfärg med fallback
                font.pixelSize: 13
                visible: connectionDialog.errorText !== ""
                Layout.alignment: Qt.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            
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
                    
                    // Ändra när protokollet ändras (visa/dölj SSH-nyckelinställningar)
                    onCurrentTextChanged: {
                        // Uppdatera port baserat på protokoll
                        portField.text = currentText === "FTP" ? "21" : "22"
                        
                        // Visa SSH-nyckelinställningar endast för SFTP
                        sshKeySection.visible = (currentText === "SFTP")
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
                        border.color: connectionDialog.serverError ? theme.error : theme.panelBorder
                    }
                    
                    // Rensa eventuella standard stylingregler
                    onFocusChanged: {
                        if (focus) {
                            color = theme.text;
                        }
                    }
                    
                    // Rensa fel när texten ändras
                    onTextChanged: connectionDialog.serverError = false
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
                        border.color: connectionDialog.portError ? theme.error : theme.panelBorder
                    }
                    
                    // Lägg till validator för heltal
                    validator: IntValidator { bottom: 1; top: 65535 }
                    
                    // Rensa fel när texten ändras
                    onTextChanged: connectionDialog.portError = false
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
            
            // Sektion för SSH-nyckelinställningar
            ColumnLayout {
                id: sshKeySection
                Layout.fillWidth: true
                spacing: 10
                visible: protocolCombo.currentText === "SFTP"
                
                // Autentiseringstyp (för SFTP)
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    
                    Text {
                        text: "Auth-metod:"
                        color: theme.text
                        font.pixelSize: 14
                        Layout.preferredWidth: 100
                    }
                    
                    ComboBox {
                        id: authTypeCombo
                        model: ["Lösenord", "SSH-nyckel", "Lösenord+Nyckel"]
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
                            text: authTypeCombo.displayText
                            color: theme.text
                            font.pixelSize: 14
                            leftPadding: 10
                            verticalAlignment: Text.AlignVCenter
                        }
                        
                        // Visa/dölj relevanta fält baserat på vald autentiseringsmetod
                        onCurrentIndexChanged: {
                            var usePassword = (currentIndex === 0 || currentIndex === 2)
                            var useKey = (currentIndex === 1 || currentIndex === 2)
                            
                            passwordField.visible = usePassword
                            keyFileSection.visible = useKey
                        }
                    }
                }
                
                // SSH-nyckelfil och lösenfras
                ColumnLayout {
                    id: keyFileSection
                    Layout.fillWidth: true
                    spacing: 10
                    visible: authTypeCombo.currentIndex > 0 // Visa om SSH-nyckel används
                    
                    // Sökväg till SSH-nyckel
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        
                        Text {
                            text: "SSH-nyckel:"
                            color: theme.text
                            font.pixelSize: 14
                            Layout.preferredWidth: 100
                        }
                        
                        TextField {
                            id: keyPathField
                            Layout.fillWidth: true
                            placeholderText: "Sökväg till SSH-nyckelfil..."
                            color: theme.text
                            readOnly: true
                            
                            background: Rectangle {
                                implicitWidth: 200
                                implicitHeight: 40
                                radius: 5
                                color: theme.listItem
                                border.width: 1
                                border.color: connectionDialog.keyPathError ? theme.error : theme.panelBorder
                            }
                        }
                        
                        Button {
                            text: "..."
                            implicitWidth: 40
                            implicitHeight: 40
                            
                            background: Rectangle {
                                radius: 5
                                color: parent.down ? Qt.darker(theme.listItem, 1.3) : 
                                       parent.hovered ? Qt.darker(theme.listItem, 1.1) : theme.listItem
                                border.width: 1
                                border.color: theme.panelBorder
                            }
                            
                            onClicked: {
                                fileDialog.open()
                            }
                        }
                    }
                    
                    // Lösenfras för SSH-nyckel
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        
                        Text {
                            text: "Lösenfras:"
                            color: theme.text
                            font.pixelSize: 14
                            Layout.preferredWidth: 100
                        }
                        
                        TextField {
                            id: keyPassphraseField
                            Layout.fillWidth: true
                            placeholderText: "Lösenfras för nyckeln (om krypterad)"
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
                        // Återställ felstatus
                        connectionDialog.errorText = ""
                        connectionDialog.serverError = false
                        connectionDialog.portError = false
                        connectionDialog.keyPathError = false
                        
                        var isValid = true
                        
                        // Validera serverfältet
                        if (serverField.text.trim() === "") {
                            connectionDialog.errorText = "Servernamn måste anges."
                            connectionDialog.serverError = true
                            isValid = false
                        }
                        
                        // Validera portfältet
                        var portNum = parseInt(portField.text)
                        if (portField.text.trim() === "" || isNaN(portNum) || portNum <= 0 || portNum > 65535) {
                            // Uppdatera felmeddelande (lägger till om serverfel redan finns)
                            connectionDialog.errorText += (connectionDialog.errorText ? "\n" : "") + "Ogiltigt portnummer (1-65535)."
                            connectionDialog.portError = true
                            isValid = false
                        }
                        
                        // Validera SSH-nyckel om sådan används
                        var authMethod = 0; // PASSWORD
                        if (protocolCombo.currentText === "SFTP" && authTypeCombo.currentIndex > 0) {
                            if (keyPathField.text.trim() === "") {
                                connectionDialog.errorText += (connectionDialog.errorText ? "\n" : "") + 
                                                            "SSH-nyckelfil måste anges när nyckelautentisering används.";
                                connectionDialog.keyPathError = true;
                                isValid = false;
                            }
                            // Sätt autentiseringsmetod
                            authMethod = authTypeCombo.currentIndex; // 1=KEY, 2=BOTH
                        }
                        
                        // Om allt är OK, anslut
                        if (isValid) {
                            backend.connectFromQmlEx(
                                protocolCombo.currentText, 
                                serverField.text.trim(), 
                                parseInt(portField.text), 
                                usernameField.text, 
                                passwordField.text,
                                authMethod,
                                keyPathField.text,
                                keyPassphraseField.text
                            );
                            connectionDialog.close();
                        }
                    }
                }
            }
        }
    }
    
    // FileDialog för att välja SSH-nyckelfil
    FileDialog {
        id: fileDialog
        title: "Välj SSH-nyckelfil"
        currentFolder: Platform.StandardPaths.standardLocations(Platform.StandardPaths.HomeLocation)[0]
        fileMode: FileDialog.OpenFile
        
        onAccepted: {
            keyPathField.text = fileDialog.selectedFile.toString().replace("file:///", "/");
            connectionDialog.keyPathError = false;
        }
    }
} 