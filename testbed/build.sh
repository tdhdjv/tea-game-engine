#!/bin/bash
# Build script for testbed

windowingSystem=$XDG_SESSION_TYPE

set echo on

mkdir -p ../bin

# Get a list of all the .c files.
cFileNames=$(find . -type f -name "*.c")

# echo "Files:" $cFilenames

assembly="testbed"
compilerFlags="-g -fdeclspec -fPIC"
# -fsanitize=address" 
# -fms-extensions 
# -Wall -Werror
includeFlags="-Isrc -I../engine/src"
linkerFlags="-L../bin/ -lengine -Wl,-rpath,./bin/"
if [ $windowingSystem == wayland ]; then
  defines="-D_DEBUG -DTEXPORT -DTPLATFORM_WINDOW_WAYLAND"
elif [ $windowingSystem == x11 ]; then
  defines="-D_DEBUG -DTEXPORT -DTPLATFORM_WINDOW_X11"
else
  echo "Error: Invalid arguments" exit
fi

echo "Building $assembly..."
echo clang $cFileNames $compilerFlags -o ../bin/$assembly $defines $includeFlags $linkerFlags
clang $cFileNames $compilerFlags -o ../bin/$assembly $defines $includeFlags $linkerFlags
