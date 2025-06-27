@echo off
:: LogReader 本地构建验证脚本
:: 模拟CI/CD中的Windows构建流程
:: Author: GeziP
:: Date: 2025-06-27

echo ========================================
echo LogReader 本地构建验证开始
echo ========================================

:: 设置变量
set BUILD_TYPE=Release
set BUILD_DIR=build_local_test

:: 检查环境
echo 检查构建环境...
where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ 错误：未找到CMake，请确保CMake在PATH中
    exit /b 1
)

where qmake >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ 错误：未找到Qt qmake，请确保Qt在PATH中
    exit /b 1
)

where windeployqt >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ 错误：未找到windeployqt，请确保Qt bin目录在PATH中
    exit /b 1
)

echo ✅ 构建环境检查通过

:: 清理旧的构建目录
if exist %BUILD_DIR% (
    echo 清理旧的构建目录...
    rmdir /s /q %BUILD_DIR%
)

:: 步骤1：编译翻译文件
echo.
echo 步骤1：编译翻译文件...
for %%f in (translations\*.ts) do (
    echo 编译 %%f
    lrelease "%%f"
    if %errorlevel% neq 0 (
        echo ❌ 翻译文件编译失败：%%f
        exit /b 1
    )
)
echo ✅ 翻译文件编译完成

:: 步骤2：CMake配置
echo.
echo 步骤2：CMake配置...
mkdir %BUILD_DIR%
cd %BUILD_DIR%
cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if %errorlevel% neq 0 (
    echo ❌ CMake配置失败
    cd ..
    exit /b 1
)
echo ✅ CMake配置完成

:: 步骤3：编译项目
echo.
echo 步骤3：编译项目...
cmake --build . --config %BUILD_TYPE%
if %errorlevel% neq 0 (
    echo ❌ 项目编译失败
    cd ..
    exit /b 1
)
echo ✅ 项目编译完成

:: 步骤4：检查可执行文件
echo.
echo 步骤4：检查可执行文件...
if exist "%BUILD_TYPE%\LogReader.exe" (
    echo ✅ 可执行文件生成成功：%BUILD_TYPE%\LogReader.exe
) else if exist "LogReader.exe" (
    echo ✅ 可执行文件生成成功：LogReader.exe
) else (
    echo ❌ 可执行文件未找到
    cd ..
    exit /b 1
)

:: 步骤5：部署Qt依赖
echo.
echo 步骤5：部署Qt依赖...
if exist "%BUILD_TYPE%\LogReader.exe" (
    cd %BUILD_TYPE%
    windeployqt.exe --release --no-translations --no-system-d3d-compiler --no-opengl-sw LogReader.exe
    cd ..
) else (
    windeployqt.exe --release --no-translations --no-system-d3d-compiler --no-opengl-sw LogReader.exe
)

if %errorlevel% neq 0 (
    echo ❌ Qt依赖部署失败
    cd ..
    exit /b 1
)
echo ✅ Qt依赖部署完成

:: 步骤6：复制翻译文件
echo.
echo 步骤6：复制翻译文件...
if exist "%BUILD_TYPE%" (
    copy "..\translations\*.qm" "%BUILD_TYPE%\" >nul 2>&1
) else (
    copy "..\translations\*.qm" "." >nul 2>&1
)
echo ✅ 翻译文件复制完成

:: 步骤7：创建便携版包
echo.
echo 步骤7：创建便携版包...
set PACKAGE_NAME=LogReader-Test-Windows-x64-Portable
if exist "%PACKAGE_NAME%" rmdir /s /q "%PACKAGE_NAME%"
mkdir "%PACKAGE_NAME%"

if exist "%BUILD_TYPE%" (
    xcopy "%BUILD_TYPE%\*" "%PACKAGE_NAME%\" /E /I /Q
) else (
    xcopy "*" "%PACKAGE_NAME%\" /E /I /Q /EXCLUDE:exclude_list.txt
)

copy "..\README.md" "%PACKAGE_NAME%\" >nul 2>&1
copy "..\LICENSE" "%PACKAGE_NAME%\" >nul 2>&1

echo ✅ 便携版包创建完成：%PACKAGE_NAME%

:: 步骤8：验证运行
echo.
echo 步骤8：验证程序可以启动...
echo 正在测试程序启动（5秒后自动关闭）...
start "" "%PACKAGE_NAME%\LogReader.exe"
timeout /t 5 /nobreak >nul
taskkill /f /im LogReader.exe >nul 2>&1

cd ..

echo.
echo ========================================
echo ✅ 本地构建验证完成！
echo.
echo 构建产物位置：
echo - 构建目录：%BUILD_DIR%
echo - 便携版包：%BUILD_DIR%\%PACKAGE_NAME%
echo.
echo 可以手动测试：
echo   cd %BUILD_DIR%\%PACKAGE_NAME%
echo   LogReader.exe
echo ========================================

pause 