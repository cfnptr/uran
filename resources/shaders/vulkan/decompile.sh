#!/bin/bash
cd "$(dirname "$BASH_SOURCE")"
shopt -s nullglob

spirv-cross --revision > /dev/null
status=$?

if [ $status -ne 0 ]; then
    echo "Failed to get SPIRV-CROSS version, please check if Vulkan SDK is installed."
    exit $status
fi

mkdir decompiled
status=$?

if [ $status -ne 0 ]; then
    echo "Failed to create decompiled directory."
    exit $status
fi

echo "Decompiling shaders..."

for f in *.spv
do
    spirv-cross --version 460 --no-es --vulkan-semantics "$f" --output "_decompiled/$f.txt"
    status=$?

    if [ $status -eq 0 ]; then
        echo "Decompiled \"$f\" shader."
    else
        echo "Failed to decompile \"$f\" shader."
        exit $status
    fi
done

exit 0
