#!/bin/bash
cd "$(dirname "$BASH_SOURCE")"
shopt -s nullglob

glslc --version > /dev/null
status=$?

if [ $status -ne 0 ]; then
    echo "Failed to get GLSLC version, please check if Vulkan SDK is installed."
    exit $status
fi

echo "Compiling shaders..."

for f in *.vert *.tesc *.tese *.geom *.frag *.comp *.rgen *.rahit *.rchit *.rmiss *.rint *.rcall *.task *.mesh
do
    glslc --target-env=vulkan1.2 -c -O "$f" -o "$f.spv"
    status=$?

    if [ $status -eq 0 ]; then
        echo "Compiled \"$f\" shader."
    else
        echo "Failed to compile \"$f\" shader."
        exit $status
    fi
done

exit 0
