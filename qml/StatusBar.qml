import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: statusBarRoot
    height: 25
    color: theme.panel // Använder temafärg
    border.color: theme.accent // Använder temafärg
    border.width: 1
    radius: 3

    property var theme: mainWindow.theme
    property string statusText: "Redo"
    property bool errorOccurred: false

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10

        // Statusikon (kan användas för fel/framgång)
        Rectangle {
            id: statusIcon
            width: 16
            height: 16
            anchors.verticalCenter: parent.verticalCenter
            radius: 8 // Rund ikon
            color: errorOccurred ? theme.error : theme.success
            visible: errorOccurred // Visa bara vid fel, annars "neutral"
            Layout.preferredWidth: 16
        }

        // Statustext
        Text {
            id: statusLabel
            text: statusBarRoot.statusText
            color: errorOccurred ? theme.error : theme.text // Röd text vid fel
            font.pixelSize: 12
            elide: Text.ElideRight
            Layout.fillWidth: true
            verticalAlignment: Text.AlignVCenter
        }

        // Framstegsindikator (t.ex. för överföringar)
        ProgressBar {
            id: progressBar
            visible: false // Dold som standard
            from: 0
            to: 100
            value: 50 // Exempelvärde
            Layout.preferredWidth: 150
            anchors.verticalCenter: parent.verticalCenter

            background: Rectangle {
                color: theme.listItem
                radius: 3
            }

            contentItem: Rectangle {
                color: theme.accent
                radius: 3
            }
        }

        // Ta bort QQuickImage här
        /*
        Image {
            id: transferIcon
            source: "qrc:/icons/transfer.png"
            width: 16
            height: 16
            anchors.verticalCenter: parent.verticalCenter
            visible: progressBar.visible
            Layout.rightMargin: 5
        }
        */
    }

    // Återställ felstatus efter en tid
    Timer {
        interval: 5000 // 5 sekunder
        running: errorOccurred
        repeat: false
        onTriggered: errorOccurred = false
    }
} 