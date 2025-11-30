
#!/bin/bash
# Build script for engine

windowingSystem=$XDG_SESSION_TYPE

set echo on

mkdir -p ../bin

# Get a list of all the .c files
cFileNames=$(find . -type f -name "*.c")

# echo "Files: " $cFileNames

assembly="engine"
compilerFlags="-g -shared -fdeclspec -fPIC"
#-fsanitize=address
# -Wall -Werror
includeFlags="-Isrc -I$VULKAN_SDK/include"

if [ $windowingSystem == wayland ]; then
  linkerFlags="-lvulkan -lwayland-client -lxkbcommon -L$VULKAN_SDK/lib"
  defines="-D_DEBUG -DTEXPORT -DTPLATFORM_WINDOW_WAYLAND"
elif [ $windowingSystem == x11 ]; then
  linkerFlags="-lvulkan -lxcb -lX11 -lX11-xcb -lxkbcommon -L$VULKAN_SDK/lib -L/usr/X11R6/lib"
  defines="-D_DEBUG -DTEXPORT -DTPLATFORM_WINDOW_X11"
else
  echo "Error: Invalid arguments" exit
fi

echo "Building $assembly..."
clang $cFileNames $compilerFlags -o ../bin/lib$assembly.so $defines $includeFlags $linkerFlags
