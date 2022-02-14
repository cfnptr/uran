#!/bin/bash
cd "$(dirname "$BASH_SOURCE")"

if [ -f "pack-linux/packer" ]; then
    echo "Found Pack utilities."
else
    git --version > /dev/null
    status=$?

    if [ $status -ne 0 ]; then
        echo "Failed to get Git version, please check if it's installed."
        exit $status
    fi

    echo "Cloning repository..."
    git clone --recursive https://github.com/cfnptr/pack pack
    status=$?

    if [ $status -ne 0 ]; then
        echo "Failed to clone repository."
        exit $status
    fi

    echo ""
    ./pack/build.sh
    status=$?

    if [ $status -ne 0 ]; then
        exit $status
    fi

    mkdir pack-linux
    status=$?

    if [ $status -ne 0 ]; then
        echo "Failed to make pack-linux directory."
        exit $status
    fi

    cd pack/build
    status=$?

    if [ $status -ne 0 ]; then
        echo "Failed to change directory to the build."
        exit $status
    fi

    mv packer unpacker pack-info ../../pack-linux
    status=$?

    if [ $status -ne 0 ]; then
        echo "Failed to move built utilities."
        exit $status
    fi

    cd ../..

    echo "Cleaning up..."
    rm -rf pack
    status=$?

    if [ $status -ne 0 ]; then
        echo "Failed to remove repository directory."
        exit $status
    fi
fi

echo ""
./shaders/vulkan/compile.sh
status=$?

if [ $status -ne 0 ]; then
    exit $status
fi

echo ""
echo "Packing resources..."

files=$(find * -type f -name "*.spv" -o -name "*.webp" -o -name "*.ttf")
files="$files $(find shaders/opengl/* -type f -name "*.vert" -o -name "*.frag")"
./linux/packer resources.pack $files

status=$?

if [ $status -ne 0 ]; then
    echo "Failed to pack resources."
    exit $status
fi
