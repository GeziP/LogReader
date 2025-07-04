# Local CI Test Script Usage Guide

## Overview
The `local_ci_test_ascii.bat` script performs the same quality checks as the CI/CD pipeline locally.

## What it does
1. **Sets up Qt environment** - Automatically configures Qt5 paths
2. **Checks tool availability** - Verifies clang-format, cmake, and cppcheck
3. **Code format check** - Validates code formatting with clang-format
4. **CMake configuration test** - Tests basic, testing, and coverage configurations
5. **Build test** - Attempts to build the project
6. **Translation file check** - Validates translation files

## Usage
```bash
# From project root
.\scripts\local_ci_test_ascii.bat

# From scripts directory
.\local_ci_test_ascii.bat
```

## Prerequisites
- Qt5 installed at `E:\software\QT\5.14.2\mingw73_32`
- MinGW installed at `E:\software\QT\Tools\mingw730_32`
- CMake available in PATH
- Optional: clang-format, cppcheck

## Troubleshooting

### Qt Configuration Issues
If you see "Qt5 CMake configuration not found":
1. Check Qt installation path
2. Modify the `QT5_ROOT` variable in the script
3. Ensure Qt5Config.cmake exists in the lib/cmake/Qt5 directory

### Build Issues
If CMake configuration fails:
1. Ensure MinGW is properly installed
2. Check that Qt5 is properly configured
3. Verify all required dependencies are available

### Format Issues
If code format checks fail:
1. Install clang-format
2. Run `clang-format -i <file>` to fix formatting
3. Ensure .clang-format file exists in project root

## Manual Qt Environment Setup
If the automatic setup doesn't work, you can manually set up Qt:

```batch
# Run this before the CI test
.\scripts\setup_qt_env.bat
```

## Output
The script will show:
- ✅ for passed checks
- ❌ for failed checks  
- ⚠️ for warnings/optional components

The script exits with code 0 if all checks pass, non-zero otherwise. 