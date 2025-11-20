# Installation Guide

This guide covers installation on different platforms.

## Prerequisites

- C++17 compatible compiler (GCC 8+, Clang 5+, MSVC 2019+)
- CMake 3.16 or higher
- Qt6 development libraries
- igraph library

## Platform-Specific Instructions

### macOS

```bash
# Install with Homebrew
brew install qt6 igraph cmake

# Verify installations
cmake --version
qmake6 --version
pkg-config --modversion igraph
```

### Ubuntu/Debian

```bash
# Install dependencies
sudo apt update
sudo apt install qt6-base-dev qt6-tools-dev cmake libigraph0-dev pkg-config

# Build tools
sudo apt install build-essential
```

### Windows

1. Install Qt6 from [Qt official website](https://www.qt.io/download-open-source)
2. Install [vcpkg](https://vcpkg.io/) for igraph:
   ```cmd
   vcpkg install igraph:x64-windows
   ```
3. Install CMake from [cmake.org](https://cmake.org/download/)

## Building

1. Navigate to the code directory:
   ```bash
   cd code
   ```

2. Create build directory:
   ```bash
   mkdir build
   cd build
   ```

3. Configure and build:
   ```bash
   cmake ..
   make -j$(nproc)
   ```

4. Run the application:
   ```bash
   ./bin/assignment2
   ```

## Configuration

### Environment Variables

- `COHERE_API_KEY`: Set this for AI assistant functionality

### Runtime Requirements

The application requires:
- Qt6 runtime libraries
- igraph shared library
- Network access (for AI features)
