#!/bin/bash
# Text2Image Build Script
# Copyright (c) 2025 Text2Image contributors

set -e

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}Text2Image Build Script${NC}"
echo -e "${GREEN}=====================${NC}"

# Default options
BUILD_TYPE="Release"
INSTALL_PREFIX="/usr/local"
BUILD_NODEJS=true
BUILD_TESTS=true
BUILD_EXAMPLES=true
NUM_THREADS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        --no-nodejs)
            BUILD_NODEJS=false
            shift
            ;;
        --no-tests)
            BUILD_TESTS=false
            shift
            ;;
        --no-examples)
            BUILD_EXAMPLES=false
            shift
            ;;
        --threads)
            NUM_THREADS="$2"
            shift 2
            ;;
        --help)
            echo -e "${GREEN}Usage:${NC} $0 [options]"
            echo -e "${GREEN}Options:${NC}"
            echo -e "  --debug              Build in debug mode"
            echo -e "  --prefix <path>      Set installation prefix (default: /usr/local)"
            echo -e "  --no-nodejs          Skip building Node.js bindings"
            echo -e "  --no-tests           Skip building tests"
            echo -e "  --no-examples        Skip building examples"
            echo -e "  --threads <num>      Number of threads to use for building (default: auto-detect)"
            echo -e "  --help               Show this help message"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

# Check dependencies
echo -e "${GREEN}Checking dependencies...${NC}"

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}CMake is required but not installed. Please install CMake 3.14 or higher.${NC}"
    exit 1
fi

# Check CMake version
CMAKE_VERSION=$(cmake --version | grep -oP 'cmake version \K[0-9]+\.[0-9]+')
if (( $(echo "$CMAKE_VERSION < 3.14" | bc -l) )); then
    echo -e "${RED}CMake version 3.14 or higher is required. Current version: $CMAKE_VERSION${NC}"
    exit 1
fi

# Check for C++ compiler
if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo -e "${RED}C++ compiler (g++ or clang++) is required but not installed.${NC}"
    exit 1
fi

# Check for Node.js if building Node.js bindings
if [ "$BUILD_NODEJS" = true ]; then
    if ! command -v node &> /dev/null; then
        echo -e "${YELLOW}Node.js is not installed. Skipping Node.js bindings.${NC}"
        BUILD_NODEJS=false
    else
        NODE_VERSION=$(node --version | grep -oP 'v\K[0-9]+')
        if [ "$NODE_VERSION" -lt 10 ]; then
            echo -e "${YELLOW}Node.js version 10 or higher is recommended. Current version: $(node --version).${NC}"
        fi
    fi
fi

# Create build directory
BUILD_DIR="build_${BUILD_TYPE,,}"
echo -e "${GREEN}Creating build directory: $BUILD_DIR${NC}"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
echo -e "${GREEN}Configuring with CMake...${NC}"
cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DTEXT2IMAGE_BUILD_TESTS="$BUILD_TESTS" \
    -DTEXT2IMAGE_BUILD_EXAMPLES="$BUILD_EXAMPLES" \
    -DTEXT2IMAGE_BUILD_NODEJS="$BUILD_NODEJS"

# Build the library
echo -e "${GREEN}Building Text2Image library...${NC}"
cmake --build . -- -j"$NUM_THREADS"

# Install the library
echo -e "${GREEN}Installing Text2Image library...${NC}"
sudo cmake --install .

# Build Node.js bindings if enabled
if [ "$BUILD_NODEJS" = true ]; then
    echo -e "${GREEN}Building Node.js bindings...${NC}"
    cd ../nodejs
    
    # Install Node.js dependencies
    npm install
    
    # Build the bindings
    npm run build
    
    echo -e "${GREEN}Node.js bindings built successfully!${NC}"
    echo -e "${GREEN}To test the Node.js bindings, run: npm test${NC}"
fi

echo -e "${GREEN}Build and installation completed successfully!${NC}"
echo -e "${GREEN}Text2Image library is now installed in $INSTALL_PREFIX${NC}"
echo -e "${GREEN}To use the library, include <text2image.h> in your C/C++ code and link against -ltext2image${NC}"