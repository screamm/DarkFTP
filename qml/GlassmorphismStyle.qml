import QtQuick 2.15

Rectangle {
    id: glassContainer
    
    // Egenskaper som kan anpassas utifrån
    property real blurRadius: 20
    property real glassOpacity: 0.7
    property color tintColor: "#ffffff"
    property real borderWidth: 1
    property color borderColor: Qt.rgba(1, 1, 1, 0.3)
    property real cornerRadius: 10
    
    // Glassmorfismkomponent med bakgrund, blur och border
    color: Qt.rgba(tintColor.r, tintColor.g, tintColor.b, glassOpacity)
    radius: cornerRadius
    border.width: borderWidth
    border.color: borderColor
    
    // I Qt 6 används enklare skuggning utan externa effekter
    Rectangle {
        id: shadowRect
        anchors.fill: parent
        anchors.margins: -3
        z: -1
        color: "transparent"
        radius: parent.radius + 3
        
        Rectangle {
            anchors.fill: parent
            anchors.margins: 3
            color: "#000000"
            opacity: 0.3
            radius: parent.radius - 3
        }
    }
} 