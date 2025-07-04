@echo off
:: LogReader Code Quality Local Verification Script
:: Simulates code quality check process in CI/CD
:: Author: GeziP
:: Date: 2025-06-27

setlocal enabledelayedexpansion

echo ========================================
echo LogReader Code Quality Local Verification Started
echo ========================================

:: Step 1: Code Format Check
echo.
echo Step 1: Code Format Check...
where clang-format >nul 2>&1
if %errorlevel% neq 0 (
    echo [WARNING] clang-format not found, skipping format check
    echo    Please install LLVM or Visual Studio to get clang-format
) else (
    echo Checking C++ code format...
    for /r src %%f in (*.cpp *.h) do (
        clang-format --dry-run --Werror "%%f" >nul 2>&1
        if %errorlevel% neq 0 (
            echo [ERROR] Format error: %%f
            echo    Fix with: clang-format -i "%%f"
        )
    )
    echo [SUCCESS] Code format check completed
)

:: Step 2: Translation File Check
echo.
echo Step 2: Translation File Check...
set TRANSLATION_ERROR=0
for %%f in (translations\*.ts) do (
    echo Checking translation file: %%f
    
    :: First check if the .qm file already exists
    set QM_FILE=%%~dpnf.qm
    if exist "!QM_FILE!" (
        echo    [INFO] QM file already exists: !QM_FILE!
    ) else (
        :: Try to compile the translation file
        lrelease "%%f"
        if %errorlevel% neq 0 (
            echo [ERROR] Translation file compilation error: %%f
            set TRANSLATION_ERROR=1
        ) else (
            echo    [INFO] Successfully compiled: %%f
        )
    )
)

if %TRANSLATION_ERROR%==0 (
    echo [SUCCESS] Translation file check passed
) else (
    echo [ERROR] Translation file check failed
)

:: Step 3: CMake Configuration Check
echo.
echo Step 3: CMake Configuration Check...
where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] CMake not found
    exit /b 1
)

mkdir build_test >nul 2>&1
cd build_test
cmake .. >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration failed
    cd ..
    exit /b 1
) else (
    echo [SUCCESS] CMake configuration check passed
)
cd ..
rmdir /s /q build_test >nul 2>&1

:: Step 4: Static Analysis (if available)
echo.
echo Step 4: Static Analysis Check...
where cppcheck >nul 2>&1
if %errorlevel% neq 0 (
    echo [WARNING] cppcheck not found, skipping static analysis
    echo    Consider installing cppcheck for code quality checks
) else (
    echo Running static analysis...
    cppcheck --enable=warning,style --inconclusive src\ >nul 2>&1
    if %errorlevel% neq 0 (
        echo [WARNING] Code quality issues found, run detailed check:
        echo    cppcheck --enable=all src\
    ) else (
        echo [SUCCESS] Static analysis check passed
    )
)

:: Step 5: File Structure Check
echo.
echo Step 5: File Structure Check...
set STRUCTURE_OK=1

if not exist "src\main.cpp" (
    echo [ERROR] Missing main file: src\main.cpp
    set STRUCTURE_OK=0
)

if not exist "CMakeLists.txt" (
    echo [ERROR] Missing build file: CMakeLists.txt
    set STRUCTURE_OK=0
)

if not exist "translations\" (
    echo [ERROR] Missing translations directory: translations\
    set STRUCTURE_OK=0
)

if %STRUCTURE_OK%==1 (
    echo [SUCCESS] File structure check passed
) else (
    echo [ERROR] File structure check failed
)

echo.
echo ========================================
echo Code Quality Check Completed
echo.
echo If all checks passed, the code can be submitted to CI/CD
echo ========================================

endlocal
pause 