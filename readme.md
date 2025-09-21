GLib C Project Starter

This project uses CMake, GLib.
It includes a VSCode configuration for accurate IntelliSense via clangd, so external libraries (SDL2, GLib, etc.) are resolved correctly.

Setup Instructions

1. Install dependencies

On Ubuntu/Debian:

```bash
sudo apt update
sudo apt install build-essential cmake pkg-config \
 libsdl2-dev libsdl2-ttf-dev \
 libglib2.0-dev clangd
```

2. Generate build system with CMake

From the project root (where CMakeLists.txt lives):

```bash
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build
ln -sf build/compile_commands.json compile_commands.json
```

This generates compile_commands.json for clangd and symlinks it to the project root.

3. VSCode setup

Create .vscode/settings.json in the project root:

```json
{
  "C_Cpp.intelliSenseEngine": "Disabled",
  "clangd.path": "/usr/bin/clangd"
}
```

### "C_Cpp.intelliSenseEngine": "disabled" disables Microsoft IntelliSense.

### "clangd.path": "/usr/bin/clangd" tells VSCode to use clangd instead.

Reload VSCode after creating this file.

4. Build and Run

To build:

cmake --build build

The executable will be available in the build/ folder.

Run it:

./build/MyProject

Notes

- clangd provides more accurate IntelliSense than the default MS engine because it uses your CMake build flags directly.
- No need to manually edit include paths for GLib or SDL2.
- If you add more libraries, re-run cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build and clangd will pick them up.
