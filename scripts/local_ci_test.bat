@echo off
:: LogReader 代码质量本地验证脚本
:: 模拟CI/CD中的代码质量检查流程
:: Author: GeziP
:: Date: 2025-06-27

echo ========================================
echo LogReader 代码质量本地验证开始
echo ========================================

:: 步骤1：代码格式检查
echo.
echo 步骤1：代码格式检查...
where clang-format >nul 2>&1
if %errorlevel% neq 0 (
    echo ⚠️  警告：未找到clang-format，跳过格式检查
    echo    请安装LLVM或Visual Studio以获得clang-format
) else (
    echo 检查C++代码格式...
    for /r src %%f in (*.cpp *.h) do (
        clang-format --dry-run --Werror "%%f" >nul 2>&1
        if %errorlevel% neq 0 (
            echo ❌ 格式错误：%%f
            echo    运行修复：clang-format -i "%%f"
        )
    )
    echo ✅ 代码格式检查完成
)

:: 步骤2：翻译文件检查
echo.
echo 步骤2：翻译文件检查...
set TRANSLATION_ERROR=0
for %%f in (translations\*.ts) do (
    echo 检查翻译文件：%%f
    lrelease "%%f" >nul 2>&1
    if %errorlevel% neq 0 (
        echo ❌ 翻译文件错误：%%f
        set TRANSLATION_ERROR=1
    )
)

if %TRANSLATION_ERROR%==0 (
    echo ✅ 翻译文件检查通过
) else (
    echo ❌ 翻译文件检查失败
)

:: 步骤3：CMake配置检查
echo.
echo 步骤3：CMake配置检查...
where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ 错误：未找到CMake
    exit /b 1
)

mkdir build_test >nul 2>&1
cd build_test
cmake .. >nul 2>&1
if %errorlevel% neq 0 (
    echo ❌ CMake配置失败
    cd ..
    exit /b 1
) else (
    echo ✅ CMake配置检查通过
)
cd ..
rmdir /s /q build_test >nul 2>&1

:: 步骤4：静态分析（如果可用）
echo.
echo 步骤4：静态分析检查...
where cppcheck >nul 2>&1
if %errorlevel% neq 0 (
    echo ⚠️  警告：未找到cppcheck，跳过静态分析
    echo    建议安装cppcheck进行代码质量检查
) else (
    echo 运行静态分析...
    cppcheck --enable=warning,style --inconclusive src\ >nul 2>&1
    if %errorlevel% neq 0 (
        echo ⚠️  发现代码质量问题，请运行详细检查：
        echo    cppcheck --enable=all src\
    ) else (
        echo ✅ 静态分析检查通过
    )
)

:: 步骤5：文件结构检查
echo.
echo 步骤5：文件结构检查...
set STRUCTURE_OK=1

if not exist "src\main.cpp" (
    echo ❌ 缺少主文件：src\main.cpp
    set STRUCTURE_OK=0
)

if not exist "CMakeLists.txt" (
    echo ❌ 缺少构建文件：CMakeLists.txt
    set STRUCTURE_OK=0
)

if not exist "translations\" (
    echo ❌ 缺少翻译目录：translations\
    set STRUCTURE_OK=0
)

if %STRUCTURE_OK%==1 (
    echo ✅ 文件结构检查通过
) else (
    echo ❌ 文件结构检查失败
)

echo.
echo ========================================
echo 代码质量检查完成
echo.
echo 如果所有检查都通过，说明代码可以提交到CI/CD
echo ========================================

pause 