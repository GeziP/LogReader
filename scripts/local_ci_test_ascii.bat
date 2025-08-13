@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion

echo ================================
echo   LogReader Local CI Test
echo ================================
echo.

REM Detect current directory
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

REM Setup Qt environment
echo === 0. Setup Qt Environment ===
REM Set Qt5 paths
set QT5_ROOT=E:\software\QT\5.14.2\mingw73_32
set Qt5_DIR=%QT5_ROOT%\lib\cmake\Qt5
set CMAKE_PREFIX_PATH=%QT5_ROOT%

REM Add Qt bin to PATH
set PATH=%QT5_ROOT%\bin;%PATH%

REM Add MinGW to PATH (needed for compilation)
set PATH=E:\software\QT\Tools\mingw730_32\bin;%PATH%

if exist "%Qt5_DIR%\Qt5Config.cmake" (
    echo ✅ Qt5 CMake configuration found
) else (
    echo ❌ Qt5 CMake configuration not found
    echo Please check Qt installation path: %QT5_ROOT%
)

if exist "%QT5_ROOT%\bin\qmake.exe" (
    echo ✅ qmake found
) else (
    echo ❌ qmake not found
)

echo.

REM Check tool availability
echo === 1. Check Tool Availability ===
set TOOLS_OK=1

where clang-format >nul 2>&1
if !errorlevel! neq 0 (
    echo ❌ clang-format not found
    set CLANG_FORMAT_OK=0
) else (
    echo ✅ clang-format found
    set CLANG_FORMAT_OK=1
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

REM 2. Code format check
echo === 2. Code Format Check ===
if !CLANG_FORMAT_OK!==1 (
    echo Checking code format...
    set FORMAT_OK=1
    for %%f in (src\*.cpp src\*.h src\core\*.cpp src\core\*.h src\ui\*.cpp src\ui\*.h src\utils\*.cpp src\utils\*.h) do (
        if exist "%%f" (
            clang-format --dry-run --Werror "%%f" >nul 2>&1
            if !errorlevel! neq 0 (
                echo ❌ Format error in %%f
                clang-format --dry-run "%%f" 2>&1 | findstr "error:"
                set FORMAT_OK=0
                set TOOLS_OK=0
            )
        )
    )
    if !FORMAT_OK!==1 echo ✅ Code format check passed
) else (
    echo ⚠️  Skipping format check - clang-format not found
)

echo.

REM 3. CMake configuration test
echo === 3. CMake Configuration Test ===
if exist build rmdir /s /q build
mkdir build
cd build

echo Testing basic configuration...
cmake .. -DCMAKE_BUILD_TYPE=Debug -G "MinGW Makefiles" >cmake_config.log 2>&1
if !errorlevel! neq 0 (
    echo ❌ CMake configuration failed
    type cmake_config.log
    set TOOLS_OK=0
    goto :end
) else (
    echo ✅ CMake configuration passed
)

echo Testing with ENABLE_TESTING...
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTING=ON -G "MinGW Makefiles" >cmake_test.log 2>&1
if !errorlevel! neq 0 (
    echo ❌ CMake with testing failed
    type cmake_test.log
    set TOOLS_OK=0
) else (
    echo ✅ CMake with testing passed
)

echo Testing with ENABLE_COVERAGE...
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTING=ON -DENABLE_COVERAGE=ON -G "MinGW Makefiles" >cmake_coverage.log 2>&1
if !errorlevel! neq 0 (
    echo ❌ CMake with coverage failed
    type cmake_coverage.log
    set TOOLS_OK=0
) else (
    echo ✅ CMake with coverage passed
)

echo.

REM 4. Build test
echo === 4. Build Test ===
cmake --build . >build.log 2>&1
if !errorlevel! neq 0 (
    echo ❌ Build failed
    type build.log
    set TOOLS_OK=0
) else (
    echo ✅ Build passed
)

echo.

REM 5. Translation file check
echo === 5. Translation File Check ===
cd ..
for %%f in (translations\*.ts) do (
    if exist "%%f" (
        echo Checking %%f...
        REM Basic XML format check
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