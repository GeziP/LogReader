# LogReader 开发工具安装指南

本指南将帮助您在D盘安装LogReader项目所需的开发工具。

## 安装流程概述

安装过程只需一个步骤：
- **在D盘安装开发工具**：使用 `setup_windows_d_ascii.ps1` 脚本

## 详细步骤

### 在D盘安装开发工具

此步骤将在D盘安装所有开发工具。

```powershell
# 以管理员身份运行PowerShell
cd 项目根目录
.\scripts\setup_windows_d_ascii.ps1
```

脚本将：
- 在D盘创建DevTools目录结构
- 安装开发工具到D盘（LLVM, Cppcheck, Python, vcpkg）
- 设置环境变量指向D盘工具
- 安装基本依赖（pre-commit, GSL等）

## 常见问题

### 下载失败问题

如果在使用脚本时遇到下载失败的问题（如403错误、SSL/TLS错误等），请手动下载以下文件并放置在相应位置：

- LLVM: https://github.com/llvm/llvm-project/releases/download/llvmorg-16.0.0/LLVM-16.0.0-win64.exe
- Cppcheck: https://github.com/danmar/cppcheck/releases/download/2.12.0/cppcheck-2.12.0-x64-Setup.msi
- Python: https://www.python.org/ftp/python/3.11.5/python-3.11.5-amd64.exe
- vcpkg: https://github.com/microsoft/vcpkg/archive/refs/heads/master.zip

### 目录已存在但为空

脚本会检查目录是否为空或缺少关键文件，并在需要时重新安装。

### 权限问题

所有脚本都需要管理员权限才能正常运行。如果遇到权限问题，请确保以管理员身份运行PowerShell。

## 安装后验证

安装完成后，请验证以下内容：

1. 检查环境变量是否正确指向D盘工具
2. 验证各工具是否可以正常运行：
   - `clang --version`
   - `cppcheck --version`
   - `python --version`
   - `vcpkg version`
3. 确认项目可以正常构建和运行 