# DarkFTP

![DarkFTP Logo](path/to/logo.png)

A modern, elegant and user-friendly FTP/SFTP client focused on design and usability.

## Screenshot

![DarkFTP Screenshot](DarkFTP_main_view.png)

## Features

- **Dual-pane Interface** - Easily transfer files between local and remote servers
- **Drag-and-drop Support** - Intuitive file transfers between local computer and remote server
- **FTP & SFTP Support** - Connect via standard FTP or secure SFTP protocols
- **Visual Transfer Indicators** - Clear progress visualization for all file operations
- **Customizable Themes** - Choose between Dark, Retro Blue, Steampunk, and Hacker themes
- **Connection Management** - Store and organize connection profiles for quick access
- **Detailed Logging** - Comprehensive activity tracking for all transfers and operations

## Installation

### Windows
1. Download the latest release from [Releases](https://github.com/screamm/DarkFTP/releases)
2. Extract all files from the ZIP archive
3. Run `DarkFTP.exe`

### macOS
1. Download the latest release from [Releases](https://github.com/screamm/DarkFTP/releases)
2. Mount the .dmg file
3. Drag DarkFTP to your Applications folder

### From Source
Requirements:
1. Qt 6.8.3 or later
2. CMake 3.16 or later
3. C++17-compatible compiler

```bash
# Clone repository
git clone https://github.com/screamm/DarkFTP.git
cd DarkFTP

# Configure and build
mkdir build && cd build
cmake ..
cmake --build . --config Release

# Deploy (Windows)
windeployqt DarkFTP.exe

# Deploy (macOS)
macdeployqt DarkFTP.app
```

## Usage

### Connecting to a Server
1. Click "Connect..." or use `Ctrl+N`
2. Enter server information:
   - Server address (e.g., ftp.example.com)
   - Username
   - Password
   - Port (default: 21 for FTP, 22 for SFTP)
   - Select connection type (FTP/SFTP)
3. Click "Connect"

### Transferring Files
- **Drag and drop**: Drag files between local and remote panels
- **Buttons**: Select a file and click "Upload" or "Download"
- **Context menu**: Right-click on a file for transfer options

### Changing Themes
1. Go to "View" > "Theme"
2. Select from available themes

## Technical Details

DarkFTP is built with:
- Qt 6.8.3 - GUI framework
- C++17 - Programming language
- CMake - Build system

## Contributing

Contributions are welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) for details.

## License

This project is licensed under the MIT License - see [LICENSE](LICENSE) file for details.

## Nytt Glassmorfism-gränssnitt

DarkFTP har nu ett banbrytande, modernt användargränssnitt baserat på glassmorfism-designprinciper. Den nya designen ger en visuellt tilltalande upplevelse med frostat glas-effekt, subtil skuggning och djupkänsla.

### Nya funktioner i Glassmorfism-UI:

- **Frostat glas-effekt** - Halvtransparenta paneler med elegant blur-effekt
- **Animerade överföringar** - Visuellt tilltalande framstegsindikatorer
- **Responsiv layout** - Anpassar sig dynamiskt till fönsterstorlek
- **Elegant dra-och-släpp** - Förbättrad DnD-funktionalitet med visuell återkoppling
- **Tema-stöd** - Modern färgpalett med stöd för anpassade teman

### Tekniska förbättringar:

- Implementerad med Qt QML och QtGraphicalEffects
- Material Design-inspirerade kontroller
- Förbättrad prestanda genom att använda Qt Quick istället för Widgets
- Separerad design från affärslogik

### Framtida utveckling:

- Utveckla fler anpassade teman (ljust/mörkt/anpassat)
- Implementera temaväxlare direkt i gränssnittet
- Lägga till fler animationer och övergångar
- Förbättra tillgänglighet och användarupplevelse

För att testa det nya gränssnittet, bygg projektet med Qt 6.8.3 eller senare med stöd för Quick och QuickControls2-modulerna. Se 'qml/README.md' för mer information om designprinciperna och implementationen.



