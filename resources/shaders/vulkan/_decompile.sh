#!/bin/bash

shopt -s nullglob
cd $(dirname "$BASH_SOURCE")

if ! spirv-cross --revision ; then
    echo "Failed to get SPIRV-CROSS version, please check if Vulkan SDK is installed."
    exit
fi

mkdir _decompiled

echo ""
echo "Decompiling shaders..."

for f in *.spv
do
    if spirv-cross --version 460 --no-es --vulkan-semantics $f --output _decompiled/$f.txt ; then
        echo "Decompiled \"$f\" shader."
    else
        echo "Failed to decompile \"$f\" shader."
        exit
    fi
done
