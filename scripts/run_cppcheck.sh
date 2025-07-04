#!/usr/bin/env bash
# ci: run cppcheck with project compile database
# Usage: ./scripts/run_cppcheck.sh <build_dir>
set -euo pipefail

BUILD_DIR=${1:-build}
COMPILE_CMD="$BUILD_DIR/compile_commands.json"

if [ ! -f "$COMPILE_CMD" ]; then
  echo "compile_commands.json not found in $BUILD_DIR" >&2
  exit 1
fi

cppcheck \
  --enable=all \
  --inconclusive \
  --std=c++17 \
  --project="$COMPILE_CMD" \
  --error-exitcode=1 \
  -j $(nproc || sysctl -n hw.ncpu || echo 2) 