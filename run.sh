#!/bin/bash
set -e

mkdir -p build
cd build

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

make

ln -sf compile_commands.json ../compile_commands.json

./MyProject
# valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./MyProject


cd ..
