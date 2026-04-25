#!/bin/bash

# MoonRender macOS Metal 启动脚本

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/Builds/CMakeOutput/Bin/Debug"
APP_NAME="MoonRender"

if [ ! -f "$BUILD_DIR/$APP_NAME" ]; then
    echo "Error: $APP_NAME not found at $BUILD_DIR/$APP_NAME"
    echo "Please build first: cmake --build Builds/CMake/macos-debug --config Debug"
    exit 1
fi

cd "$PROJECT_ROOT"

# 启动应用，使用 Metal 后端
exec "$BUILD_DIR/$APP_NAME" --gfx=metal "$@"
