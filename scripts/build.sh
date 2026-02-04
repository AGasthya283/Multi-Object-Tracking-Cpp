#!/bin/bash

# Build script for Multi-Object Tracking project

echo "Building Multi-Object Tracking System..."

# Create build directory
mkdir -p build
cd build

# Run CMake
echo "Running CMake..."
cmake ..

# Build the project
echo "Compiling..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Executable created: ./build/mot_tracker"
else
    echo "Build failed!"
    exit 1
fi
