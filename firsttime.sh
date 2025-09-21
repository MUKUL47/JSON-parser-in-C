#!/bin/bash
set -e

echo "Installing dependencies..."
sudo apt update || true
sudo apt install build-essential cmake pkg-config \
 libsdl2-dev libsdl2-ttf-dev \
 libglib2.0-dev clangd

echo "Generating CMake build system..."
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build

echo "Linking compile_commands.json..."
ln -sf build/compile_commands.json compile_commands.json

sudo apt install clang-format

echo "Setup complete!"
