#!/bin/bash
cd $(dirname "$BASH_SOURCE")

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

cd pack/build
mv packer unpacker pack-info ../..
status=$?

if [ $status -ne 0 ]; then
    echo "Failed to move utilities."
    exit $status
fi

cd ../..
rm -rf pack
status=$?

if [ $status -ne 0 ]; then
    echo "Failed to remove repository directory."
    exit $status
fi

exit 0
