# LogReader - 专业日志分析工具

<div align="center">

![LogReader](docs/screenshot.png)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Qt](https://img.shields.io/badge/Qt-5.x%2B-green.svg)](https://www.qt.io/)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20macOS%20%7C%20Linux-blue.svg)](#)
[![Quality CI](https://github.com/yourusername/LogReader/actions/workflows/quality.yml/badge.svg)](https://github.com/yourusername/LogReader/actions/workflows/quality.yml)
[![Codecov](https://codecov.io/gh/yourusername/LogReader/branch/main/graph/badge.svg)](https://codecov.io/gh/yourusername/LogReader)
[![Build Status](https://img.shields.io/badge/Build-Passing-brightgreen.svg)](#)

**🌍 语言版本：[中文](README.md) | [English](docs/README_EN.md)**

*一款功能强大、易于使用的跨平台日志分析工具，专为开发者和系统管理员设计*

</div>

## ✨ 功能特性

### 🎯 核心功能
| 功能 | 描述 | 状态 |
|------|------|------|
| 📄 **大文件支持** | 高效处理GB级大型日志文件 | ✅ |
| ⏰ **时间范围筛选** | 精确选择时间段，快速定位关键日志 | ✅ |
| 🏷️ **多维度筛选** | 支持日志等级、模块名称等多重筛选条件 | ✅ |
| 🔍 **实时搜索** | 关键词高亮显示，快速导航到目标日志 | ✅ |
| 📊 **智能展示** | 可折叠日志段，聚焦重要信息 | ✅ |

### 🚀 高级功能
| 功能 | 描述 | 状态 |
|------|------|------|
| 📤 **多格式导出** | 支持TXT、CSV、JSON格式，满足不同需求 | ✅ |
| 🧠 **智能记忆** | 记住上次路径、导出位置和格式选择 | ✅ |
| 🎛️ **批量导出** | 一键导出多种格式，提升工作效率 | ✅ |
| 🌐 **多编码支持** | 支持UTF-8、GBK等多种字符编码 | ✅ |


## 📸 功能展示

<details>
<summary>点击查看详细截图</summary>

### 主界面
![主界面](docs/screenshot.png)

### 筛选功能
- 支持时间范围、日志等级、模块名称多重筛选
- 实时显示过滤结果统计

### 导出功能
- 多格式同时导出：TXT、CSV、JSON
- 智能字段选择和预览
- 进度条显示导出状态

</details>

## 🚀 快速开始

### 1️⃣ 打开日志文件
```
文件 → 打开日志文件 → 选择.log/.txt文件
```

### 2️⃣ 设置筛选条件
- 📅 选择时间范围
- 🏷️ 勾选需要的日志等级（DEBUG、INFO、WARN、ERROR）
- 📂 选择关注的模块

### 3️⃣ 执行筛选
```
点击 "筛选" 按钮 → 查看过滤结果
```

### 4️⃣ 导出结果
```
文件 → 导出筛选结果 → 选择格式和字段 → 保存
```

## 📦 安装说明

> 📖 **详细构建指南**: [中文版](BUILD_CN.md) | [English](BUILD_EN.md)

### 系统要求
- **操作系统**: Windows 10+, macOS 10.14+, Linux (Ubuntu 18.04+)
- **Qt版本**: 5.12 或更高版本 (支持Qt5/Qt6)
- **编译器**: GCC 7+, Clang 6+, MSVC 2017+

### 从源码构建

#### 🪟 Windows
```bash
# 克隆仓库
git clone https://github.com/yourusername/LogReader.git
cd LogReader

# 使用Qt Creator打开LogReader.pro文件
# 或者使用命令行构建：
qmake
nmake  # 或 mingw32-make
```

#### 🐧 Linux
```bash
# 安装依赖
sudo apt update
sudo apt install qt5-default qttools5-dev cmake build-essential

# 构建项目
git clone https://github.com/yourusername/LogReader.git
cd LogReader
mkdir build && cd build
cmake ..
make -j$(nproc)
```

#### 🍎 macOS
```bash
# 使用Homebrew安装Qt
brew install qt5

# 构建项目
git clone https://github.com/yourusername/LogReader.git
cd LogReader
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

## 💡 使用技巧

### 🔧 日志格式支持
LogReader支持以下标准日志格式：
```
[2025-06-27 08:36:19.123] [INFO] [ModuleName] : 日志内容
[2025-06-27 08:36:19] [ERROR] [Database] : 连接失败
```

### ⚡ 性能优化建议
- 📊 **大文件处理**: 建议先设置时间范围后再打开大文件
- 🔄 **筛选策略**: 从粗筛选到细筛选，逐步缩小范围  
- 💾 **导出优化**: 大量数据导出时选择合适的格式（CSV最快）

### 🎯 实用场景
- **🐛 调试应用**: 快速定位ERROR和WARN级别的日志
- **📊 性能分析**: 筛选特定模块的性能日志
- **📈 数据分析**: 导出CSV格式用于Excel或其他分析工具
- **📋 报告生成**: 导出JSON格式用于自动化报告

## 🛠️ 开发者指南

### 项目结构
```
LogReader/
├── src/                  # 源代码目录
│   ├── main.cpp          # 应用程序入口
│   ├── ui/               # 用户界面模块
│   │   ├── logviewer.cpp/.h      # 主窗口实现
│   │   └── exportdialog.cpp/.h   # 导出配置对话框
│   ├── core/             # 核心逻辑模块
│   │   ├── logentry.h            # 日志条目数据结构
│   │   └── logexporter.cpp/.h    # 导出功能实现
│   └── utils/            # 工具类模块
│       └── appsettings.cpp/.h    # 应用设置管理
├── resources/            # 资源文件目录
│   ├── icons/            # 图标资源
│   └── resources.qrc     # Qt资源文件
├── docs/                 # 文档和截图
├── translations/         # 国际化文件
├── CMakeLists.txt        # CMake构建配置
└── LogReader.pro         # Qt工程文件
```

### 技术栈
- **UI框架**: Qt 5.x/6.x (Widgets)
- **构建系统**: qmake / CMake
- **编程语言**: C++17

### 贡献指南
1. 🍴 Fork本仓库
2. 🌱 创建功能分支 (`git checkout -b feature/amazing-feature`)
3. 💾 提交更改 (`git commit -m 'Add amazing feature'`)
4. 📤 推送到分支 (`git push origin feature/amazing-feature`)
5. 🔄 创建Pull Request

## 📄 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 🤝 致谢

- **Qt团队** - 提供优秀的跨平台框架
- **开源社区** - 提供灵感和技术支持
- **所有贡献者** - 感谢每一个改进建议和代码贡献

## 📞 联系方式

- 📧 邮箱: [shapigege@gmail.com](mailto:shapigege@gmail.com)
- 🐙 GitHub: [项目地址](https://github.com/yourusername/LogReader)
- 🌟 如果这个项目对您有帮助，请给个Star ⭐

---

<div align="center">

**Made with ❤️ by LogReader Team**

*专业的日志分析，从LogReader开始*

</div>


