# LogReader - Professional Log Analysis Tool

<div align="center">

![LogReader](screenshot.png)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Qt](https://img.shields.io/badge/Qt-5.x%2B-green.svg)](https://www.qt.io/)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20macOS%20%7C%20Linux-blue.svg)](#)
[![Build Status](https://img.shields.io/badge/Build-Passing-brightgreen.svg)](#)

**🌍 Languages: [中文](../README.md) | [English](README_EN.md)**

*A powerful, user-friendly cross-platform log analysis tool designed for developers and system administrators*

</div>

## ✨ Features

### 🎯 Core Features
| Feature | Description | Status |
|---------|-------------|--------|
| 📄 **Large File Support** | Efficiently handles GB-sized log files | ✅ |
| ⏰ **Time Range Filtering** | Precise time period selection for quick key log location | ✅ |
| 🏷️ **Multi-dimensional Filtering** | Support for log level, module name and other multiple filter conditions | ✅ |
| 🔍 **Real-time Search** | Keyword highlighting and quick navigation to target logs | ✅ |
| 📊 **Smart Display** | Collapsible log segments to focus on important information | ✅ |

### 🚀 Advanced Features
| Feature | Description | Status |
|---------|-------------|--------|
| 📤 **Multi-format Export** | Support for TXT, CSV, JSON formats to meet different needs | ✅ |
| 🧠 **Smart Memory** | Remember last paths, export locations and format selections | ✅ |
| 🎛️ **Batch Export** | One-click export to multiple formats for improved efficiency | ✅ |
| 🌐 **Multi-encoding Support** | Support for UTF-8, GBK and other character encodings | ✅ |


## 📸 Feature Showcase

<details>
<summary>Click to view detailed screenshots</summary>

### Main Interface
![Main Interface](screenshot.png)

### Filtering Features
- Support for time range, log level, and module name multi-filtering
- Real-time display of filter result statistics

### Export Features
- Multi-format simultaneous export: TXT, CSV, JSON
- Smart field selection and preview
- Progress bar showing export status

</details>

## 🚀 Quick Start

### 1️⃣ Open Log File
```
File → Open Log File → Select .log/.txt file
```

### 2️⃣ Set Filter Conditions
- 📅 Select time range
- 🏷️ Check desired log levels (DEBUG, INFO, WARN, ERROR)
- 📂 Select modules of interest

### 3️⃣ Execute Filtering
```
Click "Filter" button → View filtered results
```

### 4️⃣ Export Results
```
File → Export Filtered Results → Select format and fields → Save
```

## 📦 Installation

### System Requirements
- **Operating System**: Windows 10+, macOS 10.14+, Linux (Ubuntu 18.04+)
- **Qt Version**: 5.12 or higher
- **Compiler**: GCC 7+, Clang 6+, MSVC 2017+

### Build from Source

#### 🪟 Windows
```bash
# Clone repository
git clone https://github.com/yourusername/LogReader.git
cd LogReader

# Open LogReader.pro file with Qt Creator
# Or build using command line:
qmake
nmake  # or mingw32-make
```

#### 🐧 Linux
```bash
# Install dependencies
sudo apt update
sudo apt install qt5-default qttools5-dev cmake build-essential

# Build project
git clone https://github.com/yourusername/LogReader.git
cd LogReader
mkdir build && cd build
cmake ..
make -j$(nproc)
```

#### 🍎 macOS
```bash
# Install Qt using Homebrew
brew install qt5

# Build project
git clone https://github.com/yourusername/LogReader.git
cd LogReader
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

## 💡 Usage Tips

### 🔧 Supported Log Formats
LogReader supports the following standard log formats:
```
[2025-06-27 08:36:19.123] [INFO] [ModuleName] : Log content
[2025-06-27 08:36:19] [ERROR] [Database] : Connection failed
```

### ⚡ Performance Optimization Tips
- 📊 **Large File Handling**: Recommend setting time range before opening large files
- 🔄 **Filtering Strategy**: From coarse to fine filtering, gradually narrow down the scope
- 💾 **Export Optimization**: Choose appropriate format for large data exports (CSV is fastest)

### 🎯 Practical Use Cases
- **🐛 Debug Applications**: Quickly locate ERROR and WARN level logs
- **📊 Performance Analysis**: Filter performance logs for specific modules
- **📈 Data Analysis**: Export CSV format for Excel or other analysis tools
- **📋 Report Generation**: Export JSON format for automated reporting

## 🛠️ Developer Guide

### Project Structure
```
LogReader/
├── src/                  # Source code directory
│   ├── main.cpp          # Application entry point
│   ├── ui/               # User interface module
│   │   ├── logviewer.cpp/.h      # Main window implementation
│   │   └── exportdialog.cpp/.h   # Export configuration dialog
│   ├── core/             # Core logic module
│   │   ├── logentry.h            # Log entry data structure
│   │   └── logexporter.cpp/.h    # Export functionality
│   └── utils/            # Utility classes module
│       └── appsettings.cpp/.h    # Application settings management
├── resources/            # Resource files directory
│   ├── icons/            # Icon resources
│   └── resources.qrc     # Qt resource file
├── docs/                 # Documentation and screenshots
├── translations/         # Internationalization files
├── CMakeLists.txt        # CMake build configuration
└── LogReader.pro         # Qt project file
```

### Technology Stack
- **UI Framework**: Qt 5.x/6.x (Widgets)
- **Build System**: qmake / CMake
- **Programming Language**: C++17

### Contributing Guidelines
1. 🍴 Fork the repository
2. 🌱 Create a feature branch (`git checkout -b feature/amazing-feature`)
3. 💾 Commit your changes (`git commit -m 'Add amazing feature'`)
4. 📤 Push to the branch (`git push origin feature/amazing-feature`)
5. 🔄 Create a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](../LICENSE) file for details.

## 🤝 Acknowledgments

- **Qt Team** - For providing excellent cross-platform framework
- **Open Source Community** - For inspiration and technical support
- **All Contributors** - Thanks for every improvement suggestion and code contribution

## 📞 Contact

- 📧 Email: [shapigege@gmail.com](mailto:shapigege@gmail.com)
- 🐙 GitHub: [Project Repository](https://github.com/yourusername/LogReader)
- 🌟 If this project helps you, please give it a Star ⭐

---

<div align="center">

**Made with ❤️ by LogViewer Team**

*Professional log analysis starts with LogViewer*

</div> 