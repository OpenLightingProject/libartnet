# libartnet CMake Build Guide

This document describes how to build the libartnet library using CMake.

## 📋 Table of Contents

- [System Requirements](#system-requirements)
- [Quick Start](#quick-start)
- [Detailed Build Steps](#detailed-build-steps)
- [Configuration Options](#configuration-options)
- [Platform-Specific Instructions](#platform-specific-instructions)
- [Installation](#installation)
- [Using libartnet in Your Project](#using-libartnet-in-your-project)
- [Troubleshooting](#troubleshooting)

## System Requirements

- CMake 3.10 or higher
- C compiler (supporting C99 standard)
  - Linux/macOS: GCC or Clang
  - Windows: MSVC 2017+ or MinGW-w64

## Quick Start

### Linux/macOS

```bash
# Clone the repository
git clone **https://github.com/OpenLightingProject/libartnet.git**
cd libartnet

# Configure and build
cmake -S . -B build
cmake --build build

# Install (optional)
sudo cmake --install build
```

### Windows (using Visual Studio)

```cmd
# Clone the repository
git clone https://github.com/OpenLightingProject/libartnet.git
cd libartnet

# Generate Visual Studio 2022 project
cmake -S . -B build -G "Visual Studio 17 2022" -A x64

# Build Release version
cmake --build build --config Release

# Install (requires administrator privileges)
cmake --install build --config Release
```

### Windows (using MinGW-w64)

```cmd
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
cmake --install build
```

## Detailed Build Steps

### 1. Create Build Directory

It is recommended to use out-of-source build to keep the source directory clean:

```bash
mkdir build
cd build
```

### 2. Configure the Project

#### Basic Configuration

```bash
cmake ..
```

#### Specify Installation Path

```bash
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
```

#### Specify Build Type

```bash
# Debug version (with debug symbols)
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Release version (optimized)
cmake .. -DCMAKE_BUILD_TYPE=Release

# RelWithDebInfo (optimized with debug info)
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

### 3. Build

```bash
# Use default parallelism
cmake --build .

# Specify number of parallel jobs
cmake --build . -j 4

# Specify configuration on Windows
cmake --build . --config Release
```

### 4. Install

```bash
# Linux/macOS (may require sudo)
sudo cmake --install .

# Windows (requires administrator privileges)
cmake --install . --config Release

# Install to custom directory
cmake --install . --prefix /opt/libartnet
```

## Configuration Options

libartnet provides the following CMake configuration options:

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_SHARED_LIBS` | `ON` | Build shared library (OFF builds static library) |
| `ENABLE_IPV6` | `ON` | Enable IPv6 support |
| `CMAKE_INSTALL_PREFIX` | `/usr/local` | Installation path prefix |

### Usage Examples

```bash
# Build static library, disable IPv6
cmake -S . -B build -DBUILD_SHARED_LIBS=OFF -DENABLE_IPV6=OFF

# Install to /opt directory
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/opt
```

## Platform-Specific Instructions

### Linux

#### Dependencies

On Debian/Ubuntu systems:
```bash
sudo apt-get install build-essential cmake
```

On Fedora/RHEL systems:
```bash
sudo dnf install gcc gcc-c++ cmake
```

#### Build Example

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
sudo cmake --install build
```

### macOS

#### Dependencies

Install using Homebrew:
```bash
brew install cmake
```

#### Build Example

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(sysctl -n hw.ncpu)
sudo cmake --install build
```

### Windows

#### Using MSVC (Visual Studio)

1. Install Visual Studio 2017 or higher
2. Install CMake (download from https://cmake.org)

```cmd
# Generate Visual Studio solution
cmake -S . -B build -G "Visual Studio 17 2022" -A x64

# Build
cmake --build build --config Release

# Install
cmake --install build --config Release --prefix "C:\Program Files\libartnet"
```

#### Using MinGW-w64

1. Install MinGW-w64
2. Ensure mingw32-make is in PATH

```cmd
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build --prefix "C:\libartnet"
```

#### Windows-Specific Notes

- The build automatically links required Windows network libraries: `iphlpapi`, `netapi32`, `ws2_32`
- Shared library build automatically exports symbols (`WINDOWS_EXPORT_ALL_SYMBOLS`)

## Installation

After installation, libartnet will install the following files:

```
<prefix>/
├── include/artnet/
│   ├── artnet.h
│   ├── packets.h
│   └── common.h
├── lib/
│   ├── libartnet.so (Linux)
│   ├── libartnet.dylib (macOS)
│   ├── artnet.dll (Windows)
│   └── cmake/libartnet/
│       ├── libartnetConfig.cmake
│       ├── libartnetConfigVersion.cmake
│       └── libartnetTargets.cmake
└── lib/pkgconfig/
    └── libartnet.pc
```

## Using libartnet in Your Project

### Method 1: Using CMake find_package

In your project's `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyProject)

# Find libartnet
find_package(libartnet REQUIRED)

# Add executable
add_executable(myapp main.c)

# Link libartnet
target_link_libraries(myapp PRIVATE libartnet::artnet)
```

### Method 2: Using pkg-config

```cmake
find_package(PkgConfig REQUIRED)
pkg_check_modules(ARTNET REQUIRED libartnet)

add_executable(myapp main.c)
target_include_directories(myapp PRIVATE ${ARTNET_INCLUDE_DIRS})
target_link_libraries(myapp PRIVATE ${ARTNET_LIBRARIES})
```

### Code Example

```c
#include <artnet/artnet.h>
#include <stdio.h>

int main() {
    artnet_node node = artnet_new(NULL, 0);
    if (node == NULL) {
        fprintf(stderr, "Failed to create ArtNet node\n");
        return 1;
    }
    
    printf("libartnet initialized successfully\n");
    
    artnet_destroy(node);
    return 0;
}
```

## Troubleshooting

### Issue: CMake Not Found

**Solution:** Ensure CMake is installed and in PATH:
```bash
cmake --version
```

### Issue: Compiler Not Found

**Linux/macOS:**
```bash
# Install GCC
sudo apt-get install build-essential  # Debian/Ubuntu
sudo dnf install gcc gcc-c++           # Fedora/RHEL
```

**Windows:** Install Visual Studio or MinGW-w64

### Issue: IPv6 Detection Failed

If your system does not support IPv6, you can disable it:
```bash
cmake -S . -B build -DENABLE_IPV6=OFF
```

### Issue: Generated Library Not Found

Ensure the correct installation prefix is set and the library path is in the system search path:

**Linux:**
```bash
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
# Or update ldconfig
sudo ldconfig
```

**macOS:**
```bash
export DYLD_LIBRARY_PATH=/usr/local/lib:$DYLD_LIBRARY_PATH
```

**Windows:**
Add the library directory to the PATH environment variable

### Issue: Permission Denied (During Installation)

**Linux/macOS:** Use sudo or install to user directory:
```bash
cmake --install build --prefix ~/.local
```

**Windows:** Run Command Prompt as administrator

### Issue: CMakeCache Conflict

If you need to change the generator or reconfigure:
```bash
# Clear build directory
rm -rf build
mkdir build
cd build
cmake ..
```

## Cross-Compilation

### Cross-Compiling for ARM Linux

Create a toolchain file `arm-toolchain.cmake`:

```cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

set(CMAKE_FIND_ROOT_PATH /usr/arm-linux-gnueabihf)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```

Build:
```bash
cmake -S . -B build-arm -DCMAKE_TOOLCHAIN_FILE=arm-toolchain.cmake
cmake --build build-arm
```

## Uninstallation

CMake does not have a built-in uninstall command, but you can use the installation manifest:

```bash
# From build directory
cat install_manifest.txt | xargs sudo rm
```

Or if installed with a custom prefix, simply delete that directory:
```bash
sudo rm -rf /opt/libartnet
```

## Getting Help
- GitHub Issues: https://github.com/OpenLightingProject/libartnet/issues
- Original project documentation: See `README` and `INSTALL` files
