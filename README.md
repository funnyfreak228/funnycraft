# FunnyCraft

A free and open-source Minecraft Java Edition launcher built with C++ and wxWidgets.


## Features

- 🎮 **Multiple Instances** - Create and manage multiple Minecraft installations
- 👤 **Offline Accounts** - Play without a Microsoft/Mojang account
- 🎨 **Multiple Themes** - System, Light, and Dark theme options
- 📦 **All Minecraft Versions** - Support for Minecraft 1.8 through 1.21.11
- 🚀 **Lightweight** - Minimal resource usage, fast startup
- 💾 **Open Source** - Free forever, no telemetry or data collection

## Building from Source

### Requirements

- C++17 compatible compiler (GCC 8+, Clang 7+, MSVC 2019+)
- wxWidgets 3.0 or later
- nlohmann/json library
- POSIX-compliant system (Linux, macOS, Windows with MSYS2/MinGW)

### Linux (Ubuntu/Debian)

```bash
# Install dependencies
sudo apt update
sudo apt install build-essential libwxgtk3.2-dev nlohmann-json3-dev

# Clone the repository
git clone https://github.com/funnyfreak/funnycraft.git
cd funnycraft

# Build
make

# Or build manually:
g++ -std=c++17 -o build/funnycraft \
    src/main.cpp src/MainFrame.cpp src/AccountManager.cpp \
    src/InstanceManager.cpp src/MinecraftLauncher.cpp \
    src/AccountDialog.cpp src/InstanceDialog.cpp \
    src/DownloadManager.cpp src/PathUtils.cpp \
    src/ProgressDialog.cpp src/InstanceSettingsDialog.cpp \
    src/SettingsDialog.cpp \
    $(wx-config --cxxflags --libs) -Ibuild -lstdc++fs

# Run
./build/funnycraft
```

### Linux (Fedora/RHEL)

```bash
# Install dependencies
sudo dnf install gcc-c++ wxGTK3-devel json-devel

# Build
make
```

### macOS

```bash
# Install dependencies using Homebrew
brew install wxwidgets nlohmann-json

# Build
make
```

### Windows (MSYS2)

```bash
# Install MSYS2 from https://www.msys2.org/

# In MSYS2 terminal, install dependencies:
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-wxwidgets3.2 \
          mingw-w64-x86_64-nlohmann-json

# Build
make
```

## Usage

1. **First Launch**: The launcher will create necessary directories automatically
2. **Create an Instance**: Click "Instances..." → "Create Instance" and select a Minecraft version
3. **Add an Account**: Click "Manage..." next to Account dropdown to add an offline account
4. **Launch**: Select an instance and account, then click "Play!"

### Java Requirements

- **Minecraft 1.8 - 1.16.5**: Java 8
- **Minecraft 1.17**: Java 16
- **Minecraft 1.18+**: Java 17
- **Minecraft 1.20.5+**: Java 21

Install the appropriate Java version for your Minecraft version using your package manager:

```bash
# Example: Install Java 8 and Java 17 on Ubuntu
sudo apt install openjdk-8-jre openjdk-17-jre
```

### Data Locations

- **Instances**: `~/.local/share/funnycraft/instances/` (Linux)
- **Minecraft Data**: `~/.minecraft/` (shared with official launcher)
- **Config**: `~/.config/funnycraft/` (Linux)

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Built with [wxWidgets](https://www.wxwidgets.org/)
- JSON parsing with [nlohmann/json](https://github.com/nlohmann/json)
- Inspired by the open-source Minecraft community

## Disclaimer

This is an unofficial launcher and is not affiliated with Mojang Studios or Microsoft. Minecraft is a trademark of Mojang Studios.
