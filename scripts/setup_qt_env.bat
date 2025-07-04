@echo off
REM Setup Qt environment for CMake
REM This script sets up the Qt environment variables needed for CMake to find Qt5

echo Setting up Qt environment...

REM Set Qt5 paths
set QT5_ROOT=E:\software\QT\5.14.2\mingw73_32
set Qt5_DIR=%QT5_ROOT%\lib\cmake\Qt5
set CMAKE_PREFIX_PATH=%QT5_ROOT%

REM Add Qt bin to PATH
set PATH=%QT5_ROOT%\bin;%PATH%

REM Add MinGW to PATH (needed for compilation)
set PATH=E:\software\QT\Tools\mingw730_32\bin;%PATH%

echo Qt5 environment configured:
echo - Qt5_DIR: %Qt5_DIR%
echo - CMAKE_PREFIX_PATH: %CMAKE_PREFIX_PATH%
echo - Qt bin added to PATH

REM Verify Qt installation
if exist "%Qt5_DIR%\Qt5Config.cmake" (
    echo ✅ Qt5 CMake configuration found
) else (
    echo ❌ Qt5 CMake configuration not found
    echo Please check Qt installation path
)

if exist "%QT5_ROOT%\bin\qmake.exe" (
    echo ✅ qmake found
) else (
    echo ❌ qmake not found
)

echo.
echo You can now run CMake with Qt5 support
echo Example: cmake .. -DCMAKE_BUILD_TYPE=Debug 