# DarkFTP

A modern FTP/SFTP client with multiple themes and an elegant design. DarkFTP provides a user-friendly interface for file transfers while offering advanced features for both casual and professional users.

## Features

- **Dual Protocol Support**: Seamlessly connect to both FTP and SFTP servers with a unified interface
- **Multiple Themes**: Choose from five carefully designed themes to match your workflow preferences
- **File Operations**: Upload, download, and navigate through remote directories with ease
- **Connection Management**: Save and organize your frequently used connections
- **Transfer Tracking**: Monitor file transfers with a detailed progress indicator
- **Activity Logging**: Keep track of all operations with a comprehensive log window
- **Responsive UI**: Enjoy a clean and efficient user interface that adapts to your needs

## Themes

DarkFTP comes with multiple built-in themes to enhance your user experience:

- **Dark Termius**: An elegant dark theme inspired by modern terminal applications
- **Retro Blue**: A classic blue interface reminiscent of traditional file managers
- **Steampunk**: A nostalgic brown/gold theme with a vintage industrial aesthetic
- **Hacker**: The classic green-on-black terminal look for the authentic coding experience
- **Nordic**: A minimalist blue/dark theme based on the popular Nord color palette

## Installation

### Prerequisites

- Qt 6.x or later
- C++17 compatible compiler
- CMake 3.16 or later

### Building from Source

1. Clone the repository:
   ```
   git clone https://github.com/screamm/DarkFTP.git
   cd DarkFTP
   ```

2. Create a build directory:
   ```
   mkdir build && cd build
   ```

3. Configure and build the project:
   ```
   cmake ..
   cmake --build .
   ```

4. Run the application:
   ```
   ./DarkFTP
   ```

## Usage

### Connecting to a Server

1. Click on "File > Connect" or use the keyboard shortcut Ctrl+N
2. Enter your server details (hostname, username, password, port)
3. Select the connection type (FTP or SFTP)
4. Click "Connect" to establish the connection

### Transferring Files

- To upload files, select a file in the local view and click the "Upload" button
- To download files, select a file in the remote view and click the "Download" button
- Progress and status information will be displayed in the transfer tab

### Managing Connections

- Save frequently used connections via "File > Save Connection"
- Access saved connections through "File > Manage Connections"

## Development

DarkFTP is built with Qt and C++, following modern development practices. The codebase is structured around these key components:

- `MainWindow`: Core UI and application flow
- `FtpManager`: Handles FTP protocol operations
- `SftpManager`: Manages SFTP protocol operations
- Theme system: Provides a consistent look and feel across the application

## License

Copyright (c) 2024 DarkFTP Team

Licensed under the MIT License. See LICENSE file for details.

## Acknowledgments

- Inspired by professional FTP clients like FileZilla and Termius
- Built with the powerful Qt framework 