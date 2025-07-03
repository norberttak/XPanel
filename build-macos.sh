#!/bin/bash

# XPanel macOS Build Script
# Builds the XPanel plugin for macOS

set -e

echo "Building XPanel for macOS..."

# Check if we're on macOS
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "Error: This script must be run on macOS"
    exit 1
fi

# Install dependencies if needed
if ! command -v brew &> /dev/null; then
    echo "Homebrew not found. Please install Homebrew first."
    exit 1
fi

echo "Installing dependencies..."
brew install cmake lua hidapi

# Create build directory
mkdir -p build
cd build

# Configure CMake
echo "Configuring CMake..."
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PWD/../install" ..

# Build
echo "Building..."
cmake --build . --config Release

# Install
echo "Installing..."
cmake --build . --target install

echo "Build completed successfully!"
echo "Plugin installed in: $(cd .. && pwd)/install/XPanel/64/"