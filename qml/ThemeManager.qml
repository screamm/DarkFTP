pragma Singleton
import QtQuick 2.15

// ThemeManager.qml - Singleton för centraliserad temahantering
QtObject {
    id: themeManager
    
    // Aktuellt tema
    property string currentThemeName: "Terminal"
    
    // Tema-definitioner - alla färger samlade på ett ställe
    readonly property var themes: {
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
            highlight: "#2A4E80",
            error: "#ff6347",
            success: "#5cb85c"
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
            highlight: "#2D4A73",
            error: "#ff6347",
            success: "#5cb85c"
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
            highlight: "#493D6B",
            error: "#ff6347",
            success: "#5cb85c"
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
            highlight: "#3A3A3A",
            error: "#ff6347",
            success: "#5cb85c"
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
            highlight: "#2B7267",
            error: "#ff6347",
            success: "#5cb85c"
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
            highlight: "#4C0D99",
            error: "#ff6347",
            success: "#5cb85c"
        },
        "Ljus": {
            background: "#f5f5f5",
            gradient1: "#f5f5f5",
            gradient2: "#e5e5e5",
            panel: "#ffffff",
            accent: "#1976d2",  // Blå
            text: "#333333",
            header: "#eeeeee",
            listItem: "#ffffff",
            listItemAlt: "#f9f9f9",
            panelBorder: "#dddddd",
            highlight: "#e3f2fd",
            error: "#f44336",
            success: "#4caf50"
        }
    }
    
    // Returnera det aktuella temat (används för att få direktåtkomst till tema-färger)
    readonly property var theme: themes[currentThemeName]
    
    // Funktion för att byta tema
    function setTheme(themeName) {
        if (themes[themeName]) {
            currentThemeName = themeName;
        }
    }
} 