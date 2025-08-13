#!/bin/bash

# LogReader cppcheck 静态分析脚本
# 用于CI/CD和本地开发环境的代码质量检查

set -e

# 默认参数
BUILD_DIR="${1:-build}"
SOURCE_DIR="${2:-src}"
OUTPUT_FORMAT="${3:-text}"

# 检查cppcheck是否可用
if ! command -v cppcheck >/dev/null 2>&1; then
    echo "Error: cppcheck not found in PATH"
    echo "Please install cppcheck:"
    echo "  Ubuntu/Debian: sudo apt install cppcheck"
    echo "  macOS: brew install cppcheck"
    echo "  Windows: choco install cppcheck"
    exit 1
fi

# 检查源代码目录
if [ ! -d "$SOURCE_DIR" ]; then
    echo "Error: Source directory '$SOURCE_DIR' not found"
    exit 1
fi

echo "=== LogReader cppcheck Analysis ==="
echo "Source directory: $SOURCE_DIR"
echo "Build directory: $BUILD_DIR"
echo "Output format: $OUTPUT_FORMAT"
echo "=================================="

# 基础cppcheck命令
CPPCHECK_CMD="cppcheck"
CPPCHECK_OPTS=(
    "--enable=all"
    "--inconclusive"
    "--std=c++17"
    "--language=c++"
    "--platform=native"
    "--suppress=missingInclude"
    "--suppress=unusedFunction"
    "--suppress=unmatchedSuppression"
    "--inline-suppr"
    "--quiet"
)

# 根据输出格式调整选项
case $OUTPUT_FORMAT in
    "xml")
        CPPCHECK_OPTS+=("--xml" "--xml-version=2")
        OUTPUT_FILE="${BUILD_DIR}/cppcheck-report.xml"
        ;;
    "text")
        OUTPUT_FILE="${BUILD_DIR}/cppcheck-report.txt"
        ;;
    *)
        echo "Warning: Unknown output format '$OUTPUT_FORMAT', using text"
        OUTPUT_FILE="${BUILD_DIR}/cppcheck-report.txt"
        ;;
esac

# 包含路径
INCLUDE_PATHS=(
    "-I$SOURCE_DIR"
    "-I$SOURCE_DIR/core"
    "-I$SOURCE_DIR/ui"
    "-I$SOURCE_DIR/utils"
)

# 如果有CMake编译数据库，使用它
if [ -f "$BUILD_DIR/compile_commands.json" ]; then
    CPPCHECK_OPTS+=("--project=$BUILD_DIR/compile_commands.json")
    echo "Using CMake compile database: $BUILD_DIR/compile_commands.json"
else
    # 否则手动指定包含路径
    CPPCHECK_OPTS+=("${INCLUDE_PATHS[@]}")
    echo "Using manual include paths"
fi

# 创建输出目录
mkdir -p "$BUILD_DIR"

# 运行cppcheck
echo "Running cppcheck..."
if [ "$OUTPUT_FORMAT" = "xml" ]; then
    $CPPCHECK_CMD "${CPPCHECK_OPTS[@]}" "$SOURCE_DIR" 2>"$OUTPUT_FILE"
else
    $CPPCHECK_CMD "${CPPCHECK_OPTS[@]}" "$SOURCE_DIR" 2>&1 | tee "$OUTPUT_FILE"
fi

# 检查结果
if [ $? -eq 0 ]; then
    echo "✅ cppcheck analysis completed successfully"
    echo "Report saved to: $OUTPUT_FILE"
    
    # 显示简要统计
    if [ -f "$OUTPUT_FILE" ]; then
        ERROR_COUNT=$(grep -c "error" "$OUTPUT_FILE" 2>/dev/null || echo "0")
        WARNING_COUNT=$(grep -c "warning" "$OUTPUT_FILE" 2>/dev/null || echo "0")
        echo "Summary: $ERROR_COUNT errors, $WARNING_COUNT warnings"
        
        # 如果有错误，显示它们
        if [ "$ERROR_COUNT" -gt 0 ]; then
            echo ""
            echo "❌ Errors found:"
            grep "error" "$OUTPUT_FILE" || true
        fi
    fi
else
    echo "❌ cppcheck analysis failed"
    exit 1
fi

echo "cppcheck analysis completed." 