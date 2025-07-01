#!/bin/bash
set -e

# Optional: set these if your project has a specific build dir or install prefix
BUILD_DIR=build
INSTALL_PREFIX=/

# Clean previous builds (optional)
rm -rf "$BUILD_DIR"
mkdir "$BUILD_DIR"

# Configure with CMake
cmake -S . -B "$BUILD_DIR" -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX"

# Build everything
cmake --build "$BUILD_DIR" --target install

echo "Build complete! Executable should be in /bin and libraries (if any) in /lib."
