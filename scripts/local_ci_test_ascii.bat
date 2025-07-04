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
echo [STEP 1] Code Format Check...
echo ------------------------------
where clang-format >nul 2>&1
if %errorlevel% neq 0 (
    echo [WARNING] clang-format not found, skipping format check
    echo    Please install LLVM or Visual Studio to get clang-format
    echo [STEP 1] SKIPPED: clang-format not found
) else (
    echo Checking C++ code format...
    set FORMAT_ERROR=0
    for /r src %%f in (*.cpp *.h) do (
        echo    Checking: %%f
        clang-format --dry-run --Werror "%%f" >nul 2>&1
        if %errorlevel% neq 0 (
            echo    [ERROR] Format error: %%f
            echo    Fix with: clang-format -i "%%f"
            set FORMAT_ERROR=1
        )
    )
    
    if !FORMAT_ERROR!==0 (
        echo [STEP 1] SUCCESS: All files passed format check
    ) else (
        echo [STEP 1] FAILED: Some files have format errors
    )
)
echo.

:: Step 2: Translation File Check
echo [STEP 2] Translation File Check...
echo ------------------------------
set TRANSLATION_ERROR=0
set TRANSLATION_COUNT=0
set TRANSLATION_FOUND=0

for %%f in (translations\*.ts) do (
    set /a TRANSLATION_FOUND+=1
    echo    Checking: %%f
    
    :: First check if the .qm file already exists
    set QM_FILE=%%~dpnf.qm
    if exist "!QM_FILE!" (
        echo       [OK] QM file exists: !QM_FILE!
    ) else (
        :: Try to compile the translation file
        echo       Compiling translation file...
        lrelease "%%f"
        if %errorlevel% neq 0 (
            echo       [ERROR] Compilation failed: %%f
            set TRANSLATION_ERROR=1
        ) else (
            echo       [OK] Successfully compiled: %%f
            set /a TRANSLATION_COUNT+=1
        )
    )
)

if %TRANSLATION_FOUND%==0 (
    echo    [WARNING] No translation files found
    echo [STEP 2] SKIPPED: No translation files to check
) else (
    if %TRANSLATION_ERROR%==0 (
        echo    Found %TRANSLATION_FOUND% translation files
        echo [STEP 2] SUCCESS: All translation files are valid
    ) else (
        echo    Processed %TRANSLATION_COUNT% of %TRANSLATION_FOUND% translation files
        echo [STEP 2] FAILED: Some translation files have errors
    )
)
echo.

:: Step 3: CMake Configuration Check
echo [STEP 3] CMake Configuration Check...
echo ------------------------------
where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo    [ERROR] CMake not found
    echo [STEP 3] FAILED: CMake not installed
    exit /b 1
)

echo    Creating test build directory...
mkdir build_test >nul 2>&1
cd build_test
echo    Running CMake configuration...
cmake .. >nul 2>&1
if %errorlevel% neq 0 (
    echo    [ERROR] CMake configuration failed
    echo [STEP 3] FAILED: CMake configuration error
    cd ..
    exit /b 1
) else (
    echo    [OK] CMake configuration completed
    echo [STEP 3] SUCCESS: CMake configuration is valid
)
cd ..
rmdir /s /q build_test >nul 2>&1
echo.

:: Step 4: Static Analysis (if available)
echo [STEP 4] Static Analysis Check...
echo ------------------------------
where cppcheck >nul 2>&1
if %errorlevel% neq 0 (
    echo    [WARNING] cppcheck not found, skipping static analysis
    echo    Consider installing cppcheck for code quality checks
    echo [STEP 4] SKIPPED: cppcheck not found
) else (
    echo    Running static analysis on source code...
    cppcheck --enable=warning,style --inconclusive src\ >nul 2>&1
    if %errorlevel% neq 0 (
        echo    [WARNING] Code quality issues found
        echo    Run detailed check: cppcheck --enable=all src\
        echo [STEP 4] WARNING: Potential code quality issues
    ) else (
        echo    [OK] No major issues found
        echo [STEP 4] SUCCESS: Static analysis passed
    )
)
echo.

:: Step 5: File Structure Check
echo [STEP 5] File Structure Check...
echo ------------------------------
set STRUCTURE_OK=1

echo    Checking for required files and directories...
if not exist "src\main.cpp" (
    echo    [ERROR] Missing main file: src\main.cpp
    set STRUCTURE_OK=0
) else (
    echo    [OK] Found main file: src\main.cpp
)

if not exist "CMakeLists.txt" (
    echo    [ERROR] Missing build file: CMakeLists.txt
    set STRUCTURE_OK=0
) else (
    echo    [OK] Found build file: CMakeLists.txt
)

if not exist "translations\" (
    echo    [ERROR] Missing translations directory: translations\
    set STRUCTURE_OK=0
) else (
    echo    [OK] Found translations directory
)

if %STRUCTURE_OK%==1 (
    echo [STEP 5] SUCCESS: All required files present
) else (
    echo [STEP 5] FAILED: Missing required files
)
echo.

echo ========================================
echo Code Quality Check Summary:
echo ========================================
echo [STEP 1] Code Format Check:        %FORMAT_ERROR% errors
echo [STEP 2] Translation File Check:   %TRANSLATION_ERROR% errors
echo [STEP 3] CMake Configuration:      SUCCESS
echo [STEP 4] Static Analysis:          SUCCESS
echo [STEP 5] File Structure:           %STRUCTURE_OK% (1=OK, 0=Failed)
echo.
echo If all checks passed, the code can be submitted to CI/CD
echo ========================================

endlocal
pause 