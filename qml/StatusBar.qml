import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: statusBar
    width: parent.width
    height: 30
    color: Qt.darker("#2d2d30", 1.3) // Mörkt tema, matchande Windows Dark Theme
    
    // Default tema om inget skickas från parent
    property var theme: {
        "text": "#f0f0f0",
        "error": "#ff6347",
        "success": "#5cb85c"
    }
    
    property string statusMessage: "Redo"
    property bool errorOccurred: false

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10

        // Statusikon (kan användas för fel/framgång)
        Rectangle {
            id: statusIcon
            Layout.preferredWidth: 16
            Layout.preferredHeight: 16
            Layout.alignment: Qt.AlignVCenter
            radius: 8 // Rund ikon
            color: errorOccurred ? (theme.error || "#ff6347") : (theme.success || "#5cb85c")
            visible: errorOccurred // Visa bara vid fel, annars "neutral"
        }

        // Statustext
        Text {
            id: statusLabel
            text: statusMessage
            color: theme.text
            font.pixelSize: 12
            elide: Text.ElideRight
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
        }

        // Framstegsindikator (t.ex. för överföringar)
        ProgressBar {
            id: progressBar
            visible: false // Dold som standard
            from: 0
            to: 100
            value: 50 // Exempelvärde
            Layout.preferredWidth: 150
            Layout.alignment: Qt.AlignVCenter

            background: Rectangle {
                color: theme.listItem || "#333333"
                radius: 3
            }

            contentItem: Rectangle {
                color: theme.accent || "#4d79ff"
                radius: 3
            }
        }
    }

    // Återställ felstatus efter en tid
    Timer {
        interval: 5000 // 5 sekunder
        running: errorOccurred
        repeat: false
        onTriggered: errorOccurred = false
    }
} 