# LogReader CI/CD 指南

本指南解释了 LogReader 项目的持续集成 (CI) 和持续部署 (CD) 流程。

## 目录

- [概述](#概述)
- [本地验证](#本地验证)
- [预提交钩子](#预提交钩子)
- [GitHub Actions 工作流](#github-actions-工作流)
- [发布流程](#发布流程)
- [问题排查](#问题排查)

## 概述

LogReader 项目使用全面的 CI/CD 流水线来确保代码质量并自动化发布流程：

1. **本地验证**：提交前验证代码质量的脚本
2. **预提交钩子**：每次提交前自动运行的检查
3. **GitHub Actions**：在 GitHub 基础设施上运行的 CI/CD 工作流
4. **发布自动化**：自动化构建和发布流程

## 本地验证

### 本地 CI 测试脚本

项目包含一个本地 CI 测试脚本（`local_ci_test_ascii.bat`），执行与 CI 流水线相同的检查：

```bash
# 从项目根目录或 scripts 目录运行
.\scripts\local_ci_test_ascii.bat
```

此脚本检查：

1. **代码格式**：使用 clang-format 验证代码风格
2. **翻译文件**：验证翻译文件（*.ts）并将其编译为 .qm 文件
3. **CMake 配置**：确保项目可以使用 CMake 配置
4. **静态分析**：运行 cppcheck 检测潜在问题
5. **文件结构**：验证必要的文件和目录是否存在

### 运行脚本

脚本可以从以下位置运行：
- 项目根目录：`.\scripts\local_ci_test_ascii.bat`
- scripts 目录：`.\local_ci_test_ascii.bat`

它会自动检测位置并相应地调整路径。

### 环境设置

为了使脚本正常工作，确保您的 PATH 包括：
- D:\DevTools\LLVM\bin（用于 clang-format）
- D:\DevTools\Cppcheck（用于 cppcheck）
- Qt bin 目录（例如，E:\software\QT\5.14.2\mingw73_32\bin）

您可以在 PowerShell 中临时添加这些路径：
```powershell
$env:Path += ";D:\DevTools\LLVM\bin;D:\DevTools\Cppcheck;E:\software\QT\5.14.2\mingw73_32\bin"
```

## 预提交钩子

项目使用预提交钩子在每次提交前自动检查代码。

### 已安装的钩子

以下钩子在 `.pre-commit-config.yaml` 中配置：

1. **基本检查**：
   - trailing-whitespace：删除行尾空格
   - end-of-file-fixer：确保文件以换行符结束
   - check-yaml：验证 YAML 文件
   - check-added-large-files：防止提交大文件

2. **C++ 检查**：
   - clang-format：根据风格规则格式化代码
   - cppcheck：执行静态分析

3. **CMake 检查**：
   - cmakelint：验证 CMake 文件

### 使用预提交

预提交在您提交更改时自动运行。如果任何检查失败，提交将被中止。

要手动运行所有预提交检查：
```bash
pre-commit run --all-files
```

要跳过预提交钩子（不推荐）：
```bash
git commit -m "您的消息" --no-verify
```

## GitHub Actions 工作流

项目使用 GitHub Actions 进行 CI/CD 自动化。

### 质量 CI 工作流

`quality.yml` 工作流在每个拉取请求和推送到 main/develop 分支时运行：

#### 静态分析任务
- 在 Ubuntu、Windows 和 macOS 上运行
- 使用 clang-format 检查代码格式
- 使用 clang-tidy 和 cppcheck 执行静态分析

#### Sanitizers 任务
- 在 Ubuntu 和 macOS 上运行
- 使用地址和未定义行为 sanitizers 构建
- 运行测试以检测内存问题

#### 覆盖率任务
- 在 Ubuntu 上运行
- 生成代码覆盖率报告
- 将结果上传到 Codecov

### 发布工作流

`release.yml` 工作流在推送标签或手动触发时运行：

#### 构建任务
- 为 Windows、Linux 和 macOS 构建
- 编译翻译文件
- 创建特定平台的包

#### 发布任务
- 创建 GitHub 发布
- 附加构建产物
- 生成发布说明

## 发布流程

要创建新发布：

1. **更新版本**：
   - 更新相关文件中的版本号
   - 提交更改

2. **创建标签**：
   ```bash
   git tag v1.0.0  # 替换为实际版本
   git push origin v1.0.0
   ```

3. **监控工作流**：
   - 发布工作流将自动启动
   - 在 GitHub 的 Actions 标签页检查进度

4. **验证发布**：
   - 完成后，在 GitHub Releases 页面验证发布
   - 下载并测试构建产物

## 问题排查

### 常见问题

#### 本地 CI 测试失败

- **找不到 Clang-format**：确保 LLVM 已安装并在 PATH 中
- **找不到 Cppcheck**：确保 Cppcheck 已安装并在 PATH 中
- **CMake 配置失败**：检查 Qt 安装和 PATH
- **翻译文件错误**：确保 Qt 工具（lrelease）可用

#### 预提交钩子失败

- **钩子安装失败**：检查 Python 安装和权限
- **Clang-format 错误**：运行 `clang-format -i <文件>` 修复格式
- **Cppcheck 错误**：解决代码中报告的问题

#### GitHub Actions 失败

- **构建错误**：检查工作流日志获取具体错误信息
- **测试失败**：调查并修复失败的测试
- **部署问题**：验证 GitHub 权限和密钥

### 获取帮助

如果遇到持续问题：

1. 检查 GitHub Actions 中的详细日志
2. 在项目仓库中搜索类似问题
3. 创建包含问题详细信息的新 issue 