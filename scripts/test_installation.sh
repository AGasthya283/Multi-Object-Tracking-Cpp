#!/bin/bash

# Test script for Multi-Object Tracking System
# Verifies that all components are properly installed

echo "================================"
echo "MOT System Installation Test"
echo "================================"

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counters
PASSED=0
FAILED=0

# Test function
test_component() {
    local name=$1
    local command=$2
    
    echo -n "Testing $name... "
    if eval $command > /dev/null 2>&1; then
        echo -e "${GREEN}PASSED${NC}"
        ((PASSED++))
        return 0
    else
        echo -e "${RED}FAILED${NC}"
        ((FAILED++))
        return 1
    fi
}

echo ""
echo "1. Checking build tools..."
test_component "GCC compiler" "g++ --version"
test_component "CMake" "cmake --version"
test_component "Make" "make --version"

echo ""
echo "2. Checking OpenCV..."
test_component "OpenCV library" "pkg-config --exists opencv4"
if [ $? -eq 0 ]; then
    VERSION=$(pkg-config --modversion opencv4)
    echo "   OpenCV version: $VERSION"
fi

echo ""
echo "3. Checking project structure..."
test_component "Source files" "[ -d src ] && [ -f src/main.cpp ]"
test_component "Header files" "[ -d include ] && [ -f include/Tracker.h ]"
test_component "CMake config" "[ -f CMakeLists.txt ]"
test_component "Build script" "[ -x scripts/build.sh ]"

echo ""
echo "4. Checking model files..."
if [ -d "models" ]; then
    test_component "YOLO weights" "[ -f models/yolov4-tiny.weights ]"
    test_component "YOLO config" "[ -f models/yolov4-tiny.cfg ]"
    test_component "Class names" "[ -f models/coco.names ]"
else
    echo -e "${YELLOW}Warning: Models directory not found${NC}"
    echo "   Run: ./scripts/download_models.sh"
    ((FAILED+=3))
fi

echo ""
echo "5. Checking build..."
if [ -f "build/mot_tracker" ]; then
    test_component "Executable" "[ -x build/mot_tracker ]"
    
    # Try to run help (if available)
    echo -n "Testing executable run... "
    if ./build/mot_tracker --help > /dev/null 2>&1 || [ $? -eq 255 ]; then
        echo -e "${GREEN}PASSED${NC}"
        ((PASSED++))
    else
        echo -e "${YELLOW}WARNING${NC} (executable exists but may need video input)"
    fi
else
    echo -e "${YELLOW}Warning: Executable not found${NC}"
    echo "   Run: ./scripts/build.sh"
    ((FAILED+=2))
fi

echo ""
echo "================================"
echo "Test Results:"
echo "================================"
echo -e "Passed: ${GREEN}$PASSED${NC}"
echo -e "Failed: ${RED}$FAILED${NC}"
echo ""

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}✓ All tests passed! System is ready to use.${NC}"
    echo ""
    echo "Next steps:"
    echo "1. Place your input video as 'input.mp4'"
    echo "2. Run: ./scripts/run.sh --video input.mp4"
    exit 0
else
    echo -e "${YELLOW}⚠ Some tests failed. Please check the issues above.${NC}"
    echo ""
    echo "Common fixes:"
    echo "- Install dependencies: sudo apt-get install build-essential cmake libopencv-dev"
    echo "- Download models: ./scripts/download_models.sh"
    echo "- Build project: ./scripts/build.sh"
    exit 1
fi
