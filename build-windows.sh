#!/bin/bash
set -e

echo "Building Windows executable with MinGW..."

# Set up cross-compiler
export CC=x86_64-w64-mingw32-gcc
export AR=x86_64-w64-mingw32-ar
export RANLIB=x86_64-w64-mingw32-ranlib

# Build raylib for Windows
echo "Building raylib for Windows..."
cd raylib/src
make clean
make PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=STATIC \
    CC=$CC AR=$AR RANLIB=$RANLIB \
    PLATFORM_OS=WINDOWS GRAPHICS=GRAPHICS_API_OPENGL_33
cd ../..

# Create build directory
mkdir -p build

# Copy raylib library
cp raylib/src/libraylib.a build/libraylib_win.a

# Compile platform_windows.c separately (to avoid header conflicts)
echo "Compiling platform_windows.c..."
$CC -c platform_windows.c -o build/platform_windows.o \
    -D_WIN32 -DPLATFORM_WINDOWS

# Compile and link main executable
echo "Compiling main.c and linking..."
$CC -o build/idleclicker.exe main.c build/platform_windows.o \
    -D_WIN32 -DPLATFORM_WINDOWS \
    -Iraylib/src \
    -Lbuild -lraylib_win \
    -lopengl32 -lgdi32 -lwinmm \
    -static

echo "Build complete! Executable: build/idleclicker.exe"
ls -lh build/idleclicker.exe
