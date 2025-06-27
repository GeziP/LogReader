# LogReader 编译说明

## 编译环境要求
- Qt5.12 或更高版本 (支持Qt5/Qt6)
- C++17 编译器
- CMake 3.15+ (可选，也可使用qmake)

## 编译方法

### 方法1：使用Qt Creator (推荐)
1. 打开Qt Creator
2. 选择 "打开项目"
3. 选择 `LogReader.pro` 文件
4. 配置编译工具链
5. 点击 "构建" 按钮

### 方法2：使用qmake命令行
```bash
qmake LogReader.pro
make          # Linux/macOS
nmake         # Windows with MSVC
mingw32-make  # Windows with MinGW
```

### 方法3：使用CMake
```bash
cmake . -B build
cmake --build build
```

## 多语言功能说明

项目已实现完整的多语言支持：

### 功能特性
- 🌐 支持中文/英文界面切换
- ⚡ 工具栏即时语言切换，无需重启
- 💾 自动保存语言偏好设置
- 🔧 预留扩展接口，支持添加更多语言

### 使用方法
1. 在工具栏右侧找到 "语言:" 下拉框
2. 选择 "中文" 或 "English"
3. 界面立即切换语言，包括所有子界面
4. 语言偏好自动保存，重启后保持选择

### 架构设计
- **LanguageManager**: 单例模式管理语言切换
- **AppSettings**: 持久化语言配置
- **翻译文件**: 支持完整的界面元素翻译
- **动态刷新**: retranslateUI实现即时界面更新

## 添加新语言支持

1. 创建新的翻译文件：`translations/translation_xx.ts`
2. 在LanguageManager中添加语言枚举和映射
3. 更新构建文件中的翻译文件列表
4. 使用Qt Linguist工具翻译文本

## 故障排除

### 编译问题
- 确保Qt环境正确安装并配置PATH
- 检查Qt版本是否为5.12+
- 使用Qt Creator可避免大部分环境问题

### 语言切换问题
- 检查翻译文件是否正确生成(.qm文件)
- 确认翻译文件路径是否正确
- 查看控制台调试输出

### 翻译文件编译
- qmake和CMake都已配置自动编译.ts到.qm
- 构建时会自动将.qm文件复制到输出目录
- 如果手动编译：`lrelease translations/*.ts` 