@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion

:: =========================================
:: LogReader 自动代码格式化脚本
:: 使用本地DevTools中的clang-format工具
:: =========================================

echo ================================
echo   LogReader Auto Format
echo ================================

:: 设置clang-format路径
set CLANG_FORMAT_PATH=D:\DevTools\LLVM\bin\clang-format.exe

:: 检查clang-format是否存在
if not exist "%CLANG_FORMAT_PATH%" (
    echo ❌ 未找到clang-format: %CLANG_FORMAT_PATH%
    echo    请确保DevTools已正确安装
    exit /b 1
)

echo ✅ 找到clang-format: %CLANG_FORMAT_PATH%

:: 自动切换到项目根目录
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

echo 当前目录: %cd%
echo.

:: 格式化所有C++源文件
echo === 自动格式化C++源文件 ===
set FORMAT_COUNT=0

for /r %%f in (src\*.cpp src\*.h src\core\*.cpp src\core\*.h src\ui\*.cpp src\ui\*.h src\utils\*.cpp src\utils\*.h) do (
    if exist "%%f" (
        echo 格式化: %%f
        "%CLANG_FORMAT_PATH%" -i "%%f"
        set /a FORMAT_COUNT+=1
    )
)

echo.
echo ✅ 格式化完成，共处理 %FORMAT_COUNT% 个文件

:: 验证格式化结果
echo === 验证格式化结果 ===
set FORMAT_OK=1
for /r %%f in (src\*.cpp src\*.h src\core\*.cpp src\core\*.h src\ui\*.cpp src\ui\*.h src\utils\*.cpp src\utils\*.h) do (
    if exist "%%f" (
        "%CLANG_FORMAT_PATH%" --dry-run --Werror "%%f" >nul 2>&1
        if !errorlevel! neq 0 (
            echo ❌ 格式验证失败：%%f
            set FORMAT_OK=0
        )
    )
)

echo.
if !FORMAT_OK! == 1 (
    echo ✅ 所有文件格式验证通过
    echo    可以安全提交到CI
) else (
    echo ❌ 部分文件格式验证失败
    echo    请检查上方错误信息
)

echo.
echo ================================
echo   格式化流程结束
echo ================================
pause 