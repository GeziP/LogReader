# 隐藏控制台窗口和GitHub链接功能

## 任务概述
- **创建时间**: 2025-01-02
- **任务状态**: 执行中
- **优先级**: 中
- **预计完成**: 2025-01-02

## 需求描述

### 需求1：隐藏命令行黑框
用户反馈Release版本运行时会显示控制台窗口（命令行黑框），希望能够隐藏该窗口，提供更好的用户体验。

### 需求2：添加GitHub链接
在界面上添加链接跳转到 https://github.com/GeziP/LogReader ，方便用户为项目点赞。

## 技术方案

### 方案1：隐藏控制台窗口
- **CMake方案**: 在add_executable中添加WIN32标志，设置Windows子系统为Windows应用程序
- **qmake方案**: 在LogReader.pro中添加CONFIG += windows配置

### 方案2：GitHub链接
- **位置选择**: 状态栏右侧（始终可见，不占用菜单空间）
- **实现方式**: QLabel + 鼠标事件 + QDesktopServices
- **国际化**: 支持中英文显示

## 实施计划

### 步骤1：修改构建配置（隐藏控制台）
- [x] 修改CMakeLists.txt添加WIN32标志
- [x] 修改LogReader.pro添加windows配置

### 步骤2：实现GitHub链接功能
- [x] 修改logviewer.h添加相关声明
- [x] 实现状态栏GitHub链接UI
- [x] 实现点击事件处理
- [x] 添加国际化支持

### 步骤3：测试验证
- [ ] Windows环境测试控制台窗口
- [ ] GitHub链接跳转测试
- [ ] 多语言显示测试

## 涉及文件
- `CMakeLists.txt` - 构建配置
- `LogReader.pro` - qmake配置  
- `src/ui/logviewer.h` - 头文件声明
- `src/ui/logviewer.cpp` - 实现文件

## 执行记录

### 2025-01-02
- 开始任务执行
- 创建任务记录文档
- ✅ 完成CMakeLists.txt修改，添加WIN32标志隐藏控制台窗口
- ✅ 完成LogReader.pro修改，添加windows配置
- ✅ 完成logviewer.h修改，添加GitHub链接相关声明和事件过滤器
- ✅ 完成logviewer.cpp修改，实现状态栏GitHub链接UI和点击事件处理
- ✅ 添加国际化支持，GitHub链接工具提示支持中英文切换
- ✅ 修复编译错误：在logviewer.h中添加QLabel前向声明
- 🚧 代码实现完成，等待测试验证 