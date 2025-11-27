
#!/bin/bash
# Build script for engine
set echo on

mkdir -p ../bin

# Get a list of all the .c files
cFileNames=$(find . -type f -name "*.c")

# echo "Files: " $cFileNames

assembly="engine"
compilerFlags="-g -shared -fdeclspec -fPIC"
# -Wall -Werror
includeFlags="-Isrc -I$VULKAN_SDK/include"
linkerFlags="-lvulkan -lwayland-client -lxkbcommon -L$VULKAN_SDK/lib"
defines="-D_DEBUG -DTEXPORT"

echo "Building $assembly..."
clang $cFileNames $compilerFlags -o ../bin/lib$assembly.so $defines $includeFlags $linkerFlags
