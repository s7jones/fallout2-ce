# Common Development Commands

Run these commands from the repository root.

## Prerequisites

You need:

- CMake 3.19 or newer.
- A supported C++ compiler/toolchain.
- SDL2 and zlib development packages when using system dependencies.
- Original Fallout 2 game data to run the executable.

Windows builds use the **Visual Studio 17 2022** generator. Linux builds use the system SDL2 and zlib packages through the presets.

## Windows x64

Configure:

```powershell
cmake --preset windows-x64
```

Build Debug:

```powershell
cmake --build --preset windows-x64-debug
```

Build Release:

```powershell
cmake --build --preset windows-x64-release
```

The executable is created at:

```text
out/build/windows-x64/Debug/fallout2-ce.exe
out/build/windows-x64/Release/fallout2-ce.exe
```

For 32-bit Windows, use `windows-x86`, `windows-x86-debug`, and `windows-x86-release` instead.

## Linux x64

```bash
cmake --preset linux-x64-debug
cmake --build --preset linux-x64-debug
```

The executable is:

```text
out/build/linux-x64-debug/fallout2-ce
```

For a release-with-debug-information build:

```bash
cmake --preset linux-x64-release
cmake --build --preset linux-x64-release
```

The executable is:

```text
out/build/linux-x64-release/fallout2-ce
```

The x86 presets are `linux-x86-debug` and `linux-x86-release`.

## macOS

```bash
cmake --preset macos
cmake --build --preset macos-debug
```

The Xcode build output is under:

```text
out/build/macos/Debug/
```

Create the distributable DMG from the build directory with:

```bash
cd out/build/macos
cpack -C Debug
```

## Running the game

The executable expects Fallout 2 data files. Run it with the original Fallout 2 installation as the working directory, or copy the executable into that installation directory.

Example on Windows:

```powershell
cd C:\Games\Fallout2
C:\dev\fallout2-ce\out\build\windows-x64\Debug\fallout2-ce.exe
```

Example on Linux:

```bash
cd ~/Games/Fallout2
~/src/fallout2-ce/out/build/linux-x64-debug/fallout2-ce
```

## Clean rebuild

Build while cleaning first:

```bash
cmake --build --preset windows-x64-debug --clean-first
```

Replace the preset with the platform/configuration being used. If the build directory is stale or points to an unavailable compiler/toolchain, delete that preset's directory and configure again:

```text
out/build/windows-x64/
out/build/linux-x64-debug/
```

Then rerun the configure and build commands.

## Useful CMake commands

List available presets:

```bash
cmake --list-presets
```

Inspect the generated build files without building:

```bash
cmake --build --preset windows-x64-debug --target help
```

The project can also be configured manually when a preset is not suitable:

```bash
cmake -B build -D CMAKE_BUILD_TYPE=Debug
cmake --build build
```
