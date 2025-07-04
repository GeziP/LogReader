# Development Tools Installation Guide

This guide provides detailed instructions for installing the development tools required for the LogReader project on the D: drive.

## Table of Contents

- [Overview](#overview)
- [System Requirements](#system-requirements)
- [Installation Process](#installation-process)
- [Manual Installation](#manual-installation)
- [Verifying Installation](#verifying-installation)
- [Troubleshooting](#troubleshooting)

## Overview

The LogReader project requires several development tools:

- **LLVM/Clang**: For code formatting and static analysis
- **Cppcheck**: For static code analysis
- **Python**: For scripts and pre-commit hooks
- **vcpkg**: For C++ package management

This guide explains how to install these tools on the D: drive to better manage disk space.

## System Requirements

- **Operating System**: Windows 10 or later
- **Disk Space**: At least 5GB free on the D: drive
- **PowerShell**: Version 5.0 or later
- **Administrator Rights**: Required for installation

## Installation Process

### Automated Installation

The easiest way to install all required tools is to use the provided PowerShell script:

1. **Open PowerShell as Administrator**

2. **Navigate to the project root directory**
   ```powershell
   cd path\to\LogReader
   ```

3. **Run the installation script**
   ```powershell
   .\scripts\setup_windows_d_ascii.ps1
   ```

4. **Wait for completion**
   The script will:
   - Create the D:\DevTools directory structure
   - Download and install LLVM, Cppcheck, Python, and vcpkg
   - Configure environment variables
   - Install pre-commit and other dependencies

### What the Script Does

The installation script performs the following actions:

1. **Creates directory structure**:
   - D:\DevTools
   - D:\DevTools\LLVM
   - D:\DevTools\Cppcheck
   - D:\DevTools\Python
   - D:\DevTools\vcpkg

2. **Downloads and installs tools**:
   - LLVM 16.0.0 (includes clang-format and clang-tidy)
   - Cppcheck 2.12.0
   - Python 3.11.5
   - vcpkg (latest version)

3. **Sets environment variables**:
   - Adds tool paths to the system PATH
   - Configures vcpkg integration

4. **Installs dependencies**:
   - pre-commit via pip
   - Microsoft GSL via vcpkg

## Manual Installation

If the automated script fails, you can install the tools manually:

### LLVM/Clang

1. Download LLVM 16.0.0 from [LLVM Releases](https://github.com/llvm/llvm-project/releases/download/llvmorg-16.0.0/LLVM-16.0.0-win64.exe)
2. Run the installer and set the installation path to D:\DevTools\LLVM
3. Add D:\DevTools\LLVM\bin to your system PATH

### Cppcheck

1. Download Cppcheck 2.12.0 from [Cppcheck Releases](https://github.com/danmar/cppcheck/releases/download/2.12.0/cppcheck-2.12.0-x64-Setup.msi)
2. Run the installer and set the installation path to D:\DevTools\Cppcheck
3. Add D:\DevTools\Cppcheck to your system PATH

### Python

1. Download Python 3.11.5 from [Python.org](https://www.python.org/ftp/python/3.11.5/python-3.11.5-amd64.exe)
2. Run the installer and set the installation path to D:\DevTools\Python
3. Add D:\DevTools\Python and D:\DevTools\Python\Scripts to your system PATH
4. Install pre-commit: `D:\DevTools\Python\python.exe -m pip install pre-commit`

### vcpkg

1. Download vcpkg from [GitHub](https://github.com/microsoft/vcpkg/archive/refs/heads/master.zip)
2. Extract to D:\DevTools\vcpkg
3. Open PowerShell as Administrator and run:
   ```powershell
   cd D:\DevTools\vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg integrate install
   .\vcpkg install ms-gsl:x64-windows
   ```

## Verifying Installation

To verify that all tools are installed correctly:

1. **Open a new PowerShell window**

2. **Check LLVM/clang-format**
   ```powershell
   D:\DevTools\LLVM\bin\clang-format --version
   ```
   Expected output: `clang-format version 16.0.0`

3. **Check Cppcheck**
   ```powershell
   D:\DevTools\Cppcheck\cppcheck --version
   ```
   Expected output: `Cppcheck 2.12.0`

4. **Check Python**
   ```powershell
   D:\DevTools\Python\python.exe --version
   ```
   Expected output: `Python 3.11.5`

5. **Check vcpkg**
   ```powershell
   D:\DevTools\vcpkg\vcpkg version
   ```
   Expected output: Version information for vcpkg

6. **Check pre-commit**
   ```powershell
   D:\DevTools\Python\Scripts\pre-commit --version
   ```
   Expected output: `pre-commit 4.x.x`

## Troubleshooting

### Common Issues

#### Download Failures

If you encounter download failures (403 errors, SSL/TLS errors, etc.), manually download the files and place them in the appropriate locations:

- LLVM: https://github.com/llvm/llvm-project/releases/download/llvmorg-16.0.0/LLVM-16.0.0-win64.exe
- Cppcheck: https://github.com/danmar/cppcheck/releases/download/2.12.0/cppcheck-2.12.0-x64-Setup.msi
- Python: https://www.python.org/ftp/python/3.11.5/python-3.11.5-amd64.exe
- vcpkg: https://github.com/microsoft/vcpkg/archive/refs/heads/master.zip

#### Directory Exists but Empty

The script checks if directories are empty or missing key files and reinstalls when necessary.

#### Permission Issues

Make sure you're running PowerShell as Administrator.

#### Path Too Long Errors

If you encounter "Path Too Long" errors during vcpkg installation, consider moving the vcpkg directory closer to the root (e.g., D:\vcpkg).

#### Python Installation Fails

If Python installation fails, try installing it manually with the "Add Python to PATH" option unchecked, then add the paths manually.

### Getting Help

If you encounter issues not covered here:

1. Check the script logs (usually in the temp directory)
2. Create a GitHub issue with detailed information
3. Contact the project maintainers via email 