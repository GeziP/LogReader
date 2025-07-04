@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion

echo ================================
echo   LogReader Local CI Test
echo ================================
echo.

REM 检测当前目录
if exist "CMakeLists.txt" (
    set PROJECT_ROOT=%cd%
) else if exist "..\CMakeLists.txt" (
    set PROJECT_ROOT=%cd%\..
    cd /d "!PROJECT_ROOT!"
) else (
    echo ERROR: Cannot find CMakeLists.txt in current or parent directory
    exit /b 1
)

echo PROJECT_ROOT: !PROJECT_ROOT!
echo.

REM 检查工具可用性
echo === 1. 检查工具可用性 ===
set TOOLS_OK=1

where clang-format >nul 2>&1
if !errorlevel! neq 0 (
    echo ❌ clang-format not found
    set TOOLS_OK=0
) else (
    echo ✅ clang-format found
)

where cmake >nul 2>&1
if !errorlevel! neq 0 (
    echo ❌ cmake not found
    set TOOLS_OK=0
) else (
    echo ✅ cmake found
)

where cppcheck >nul 2>&1
if !errorlevel! neq 0 (
    echo ⚠️  cppcheck not found (optional)
) else (
    echo ✅ cppcheck found
)

echo.

REM 2. 代码格式检查
echo === 2. 代码格式检查 ===
if exist "%USERPROFILE%\.vscode\extensions\ms-vscode.cpptools-*\LLVM\bin\clang-format.exe" (
    echo Using VS Code clang-format...
    for /r "%USERPROFILE%\.vscode\extensions" %%i in (clang-format.exe) do (
        set CLANG_FORMAT=%%i
        goto :format_check
    )
)

:format_check
if defined CLANG_FORMAT (
    echo Checking code format...
    for %%f in (src\*.cpp src\*.h src\core\*.cpp src\core\*.h src\ui\*.cpp src\ui\*.h src\utils\*.cpp src\utils\*.h) do (
        if exist "%%f" (
            "!CLANG_FORMAT!" --dry-run --Werror "%%f" >nul 2>&1
            if !errorlevel! neq 0 (
                echo ❌ Format error in %%f
                "!CLANG_FORMAT!" --dry-run "%%f" 2>&1 | findstr "error:"
                set TOOLS_OK=0
            )
        )
    )
    if !TOOLS_OK!==1 echo ✅ Code format check passed
) else (
    echo ⚠️  Skipping format check - clang-format not found
)

echo.

REM 3. CMake配置测试
echo === 3. CMake配置测试 ===
if exist build rmdir /s /q build
mkdir build
cd build

echo Testing basic configuration...
cmake .. -DCMAKE_BUILD_TYPE=Debug >cmake_config.log 2>&1
if !errorlevel! neq 0 (
    echo ❌ CMake configuration failed
    type cmake_config.log
    set TOOLS_OK=0
    goto :end
) else (
    echo ✅ CMake configuration passed
)

echo Testing with ENABLE_TESTING...
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTING=ON >cmake_test.log 2>&1
if !errorlevel! neq 0 (
    echo ❌ CMake with testing failed
    type cmake_test.log
    set TOOLS_OK=0
) else (
    echo ✅ CMake with testing passed
)

echo Testing with ENABLE_COVERAGE...
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTING=ON -DENABLE_COVERAGE=ON >cmake_coverage.log 2>&1
if !errorlevel! neq 0 (
    echo ❌ CMake with coverage failed
    type cmake_coverage.log
    set TOOLS_OK=0
) else (
    echo ✅ CMake with coverage passed
)

echo.

REM 4. 构建测试
echo === 4. 构建测试 ===
cmake --build . >build.log 2>&1
if !errorlevel! neq 0 (
    echo ❌ Build failed
    type build.log | findstr /i "error"
    set TOOLS_OK=0
) else (
    echo ✅ Build passed
)

echo.

REM 5. 翻译文件检查
echo === 5. 翻译文件检查 ===
cd ..
for %%f in (translations\*.ts) do (
    if exist "%%f" (
        echo Checking %%f...
        REM 基本的XML格式检查
        findstr /c:"<?xml" "%%f" >nul
        if !errorlevel! neq 0 (
            echo ❌ Invalid translation file: %%f
            set TOOLS_OK=0
        ) else (
            echo ✅ Translation file OK: %%f
        )
    )
)

echo.

:end
cd "!PROJECT_ROOT!"

echo ================================
if !TOOLS_OK!==1 (
    echo ✅ All checks passed!
    echo Your code is ready for CI/CD
) else (
    echo ❌ Some checks failed
    echo Please fix the issues above
)
echo ================================

pause
exit /b !TOOLS_OK! 