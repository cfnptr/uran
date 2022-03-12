#!/bin/bash

cd $(dirname "$BASH_SOURCE")
shaders/vulkan/_compile.sh

echo ""
echo "Packing files..."

./packer editor-resources.pack \
fonts/jetbrains-mono-bold-italic.ttf \
fonts/jetbrains-mono-bold.ttf \
fonts/jetbrains-mono-italic.ttf \
fonts/jetbrains-mono-regular.ttf \
fonts/noto-sans-bold-italic.ttf \
fonts/noto-sans-bold.ttf \
fonts/noto-sans-italic.ttf \
fonts/noto-sans-regular.ttf \
shaders/opengl/panel.frag \
shaders/opengl/panel.vert \
shaders/vulkan/panel.frag.spv \
shaders/vulkan/panel.vert.spv \
shaders/opengl/text.frag \
shaders/opengl/text.vert \
shaders/vulkan/text.frag.spv \
shaders/vulkan/text.vert.spv \
