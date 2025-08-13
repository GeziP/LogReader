@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion

:: =========================================
:: LogReader 一键格式化与CI一致性检查脚本
:: 适用于 Windows + Qt + MinGW 环境，确保本地与CI一致
:: 步骤：环境变量设置 → 目录切换 → 工具检测 → 格式化 → 静态分析 → 翻译检查 → 结构检查 → 构建测试 → 总结
:: =========================================

:: === 1. 自动设置Qt和MinGW环境变量（如已设置可忽略）===
set QT_ROOT=E:\software\QT\5.14.2\mingw73_32
set Qt5_DIR=%QT_ROOT%\lib\cmake\Qt5
set CMAKE_PREFIX_PATH=%QT_ROOT%
set MINGW_BIN=E:\software\QT\Tools\mingw730_32\bin
set PATH=%QT_ROOT%\bin;%MINGW_BIN%;%PATH%

echo [INFO] 当前Qt路径: %QT_ROOT%
echo [INFO] MinGW路径: %MINGW_BIN%

:: === 2. 自动切换到项目根目录 ===
if exist "CMakeLists.txt" (
    set PROJECT_ROOT=%cd%
) else if exist "..\CMakeLists.txt" (
    cd ..
    set PROJECT_ROOT=%cd%
) else (
    echo ❌ 未在项目根目录或上级目录未找到CMakeLists.txt
    echo    请在项目根目录或scripts目录下运行本脚本
    exit /b 1
)

echo ================================
echo   LogReader Auto Format & Check
echo ================================
echo 当前目录: %cd%
echo.

:: === 3. 检查g++版本，确认MinGW生效 ===
g++ --version
if !errorlevel! neq 0 (
    echo ❌ 未检测到MinGW g++，请检查MinGW路径
    exit /b 1
)

:: === 4. 检查依赖工具 ===
set TOOLS_OK=1
where clang-format >nul 2>&1
if !errorlevel! neq 0 (
    echo ❌ 未找到 clang-format，请安装LLVM或Visual Studio
    set TOOLS_OK=0
) else (
    echo ✅ clang-format 已就绪
)
where cppcheck >nul 2>&1
if !errorlevel! neq 0 (
    echo ⚠️  未找到 cppcheck，跳过静态分析
) else (
    echo ✅ cppcheck 已就绪
)
where cmake >nul 2>&1
if !errorlevel! neq 0 (
    echo ❌ 未找到 cmake，请安装CMake
    set TOOLS_OK=0
) else (
    echo ✅ cmake 已就绪
)
where lrelease >nul 2>&1
if !errorlevel! neq 0 (
    echo ⚠️  未找到 lrelease，跳过翻译文件检查
) else (
    echo ✅ lrelease 已就绪
)
echo.

if !TOOLS_OK! neq 1 (
    echo 依赖工具缺失，部分检查将被跳过，请先安装必需工具！
)

:: === 5. 自动格式化所有C++源文件 ===
echo === 自动格式化C++源文件 ===
for /r %%f in (src\*.cpp src\*.h src\core\*.cpp src\core\*.h src\ui\*.cpp src\ui\*.h src\utils\*.cpp src\utils\*.h) do (
    if exist "%%f" (
        clang-format -i "%%f"
    )
)
echo 格式化完成。

:: === 6. 检查格式化结果 ===
echo === 检查格式化一致性 ===
set FORMAT_OK=1
for /r %%f in (src\*.cpp src\*.h src\core\*.cpp src\core\*.h src\ui\*.cpp src\ui\*.h src\utils\*.cpp src\utils\*.h) do (
    if exist "%%f" (
        clang-format --dry-run --Werror "%%f" >nul 2>&1
        if !errorlevel! neq 0 (
            echo ❌ 格式未通过：%%f
            set FORMAT_OK=0
        )
    )
)
if !FORMAT_OK! == 1 (
    echo ✅ 所有C++文件格式已统一
) else (
    echo ❌ 存在未通过格式化的文件，请手动检查上方列表
)
echo.

:: === 7. cppcheck静态分析 ===
echo === 运行cppcheck静态分析 ===
where cppcheck >nul 2>&1
if !errorlevel! neq 0 (
    echo ⚠️  跳过静态分析（未安装cppcheck）
) else (
    cppcheck --enable=all --inconclusive --std=c++17 --platform=native src/ 2>cppcheck_report.txt
    findstr /C:"[error]" cppcheck_report.txt >nul 2>&1
    if !errorlevel! == 0 (
        echo ❌ 存在cppcheck错误，详见cppcheck_report.txt
    ) else (
        echo ✅ cppcheck未发现阻断性错误
    )
)
echo.

:: === 8. 翻译文件lrelease检查 ===
echo === 检查翻译文件 ===
where lrelease >nul 2>&1
if !errorlevel! neq 0 (
    echo ⚠️  跳过翻译文件检查（未安装lrelease）
) else (
    set TRANS_OK=1
    for %%f in (translations\*.ts) do (
        lrelease "%%f" >nul 2>&1
        if !errorlevel! neq 0 (
            echo ❌ 翻译文件错误：%%f
            set TRANS_OK=0
        )
    )
    if !TRANS_OK! == 1 (
        echo ✅ 所有翻译文件检查通过
    ) else (
        echo ❌ 存在翻译文件错误，请修复后重试
    )
)
echo.

:: === 9. 关键结构检查 ===
echo === 检查关键文件结构 ===
set STRUCT_OK=1
if not exist "src\main.cpp" (
    echo ❌ 缺少主文件：src\main.cpp
    set STRUCT_OK=0
)
if not exist "CMakeLists.txt" (
    echo ❌ 缺少构建文件：CMakeLists.txt
    set STRUCT_OK=0
)
if not exist "translations\" (
    echo ❌ 缺少翻译目录：translations\
    set STRUCT_OK=0
)
if !STRUCT_OK! == 1 (
    echo ✅ 关键结构检查通过
) else (
    echo ❌ 关键结构检查失败
)
echo.

:: === 10. CMake配置/构建测试（强制MinGW Makefiles） ===
echo === CMake配置与构建测试 ===
if exist build_check rmdir /s /q build_check
mkdir build_check
cd build_check
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug >cmake_config.log 2>&1
if !errorlevel! neq 0 (
    echo ❌ CMake配置失败
    type cmake_config.log
    cd ..
    goto :end
) else (
    echo ✅ CMake配置通过
)
cmake --build . >build.log 2>&1
if !errorlevel! neq 0 (
    echo ❌ 构建失败
    type build.log
    cd ..
    goto :end
) else (
    echo ✅ 构建通过
)
cd ..
rmdir /s /q build_check >nul 2>&1

:end
echo.
echo ================================
echo   检查与格式化流程结束
echo   如有错误请根据提示修复
echo   通过后可直接提交，CI必过
echo ================================

pause 