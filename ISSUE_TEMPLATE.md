# Issue: 添加控制台窗口隐藏和GitHub链接功能

## 📋 Issue信息
- **Issue类型**: Feature Request  
- **优先级**: Medium
- **标签**: `enhancement`, `ui`, `build`
- **里程碑**: v1.2.0
- **创建时间**: 2025-01-02

## 🎯 功能需求

### 需求1：隐藏控制台窗口 🖥️
**问题描述**：
当前Windows版本的Release构建会显示命令行黑框（控制台窗口），影响用户体验。

**期望行为**：
Windows版本应该作为纯GUI应用程序运行，不显示控制台窗口。

**受影响平台**：
- Windows 10/11

### 需求2：添加GitHub链接 ⭐
**问题描述**：
用户希望在应用程序界面中能够方便地访问GitHub项目页面为项目点赞。

**期望行为**：
- 在界面适当位置添加GitHub链接
- 点击链接能够在默认浏览器中打开项目页面
- 支持国际化（中英文）

**目标URL**：
https://github.com/GeziP/LogReader

## 🔧 技术方案

### 控制台隐藏方案
- **CMake**: 在`add_executable`中添加`WIN32`标志
- **qmake**: 在`.pro`文件中添加`CONFIG += windows`

### GitHub链接方案  
- **位置**: 状态栏右侧（始终可见）
- **实现**: QLabel + 事件过滤器 + QDesktopServices
- **样式**: GitHub官方蓝色(#0969da) + 下划线
- **图标**: ⭐ 星号表示点赞

## ✅ 验收标准

### 控制台窗口隐藏
- [ ] Windows Release版本不显示控制台窗口
- [ ] Debug版本可选择性保留控制台（用于调试）
- [ ] 不影响Linux/macOS构建

### GitHub链接
- [ ] 状态栏显示GitHub链接
- [ ] 点击链接正确跳转到GitHub页面
- [ ] 支持中英文工具提示
- [ ] 链接样式符合UI设计规范
- [ ] 语言切换时链接文本正确更新

## 🧪 测试用例

### 功能测试
1. **构建测试**
   - Windows CMake构建无控制台窗口
   - Windows qmake构建无控制台窗口
   - Linux/macOS构建不受影响

2. **链接测试**
   - 点击GitHub链接能够正确跳转
   - 不同浏览器环境下链接正常工作

3. **国际化测试**
   - 中文界面：工具提示显示中文
   - 英文界面：工具提示显示英文
   - 语言切换时链接文本即时更新

## 📁 涉及文件
- `CMakeLists.txt` - 构建配置
- `LogReader.pro` - qmake配置
- `src/ui/logviewer.h` - 头文件声明
- `src/ui/logviewer.cpp` - 功能实现

## 🔗 相关资源
- [Qt Windows部署文档](https://doc.qt.io/qt-5/windows-deployment.html)
- [QDesktopServices文档](https://doc.qt.io/qt-5/qdesktopservices.html)
- [GitHub官方设计规范](https://primer.style/design/)

## 👥 负责人
- **开发者**: AI Assistant
- **审核者**: @GeziP  
- **测试者**: @GeziP

---
*这个Issue是按照企业级开发流程创建的功能需求文档* 