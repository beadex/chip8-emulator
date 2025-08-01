# Simple CHIP8 Emulator/Interpreter made with C and SDL3

## Prerequisites
- A bit knowledge of how computer work
- Get to know C syntax & build tools
- A crazy good pseudo-code [CHIP8 Guide](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/)

## Dependencies
- CMake (latest is good enough)
- SDL3

For my current workflow, I completely relied on [MSYS2](https://www.msys2.org/) on Windows. I don't have any Linux/MacOS machine lying around, so cannot test at the moment. Will do when available!

### Install
Install the MSYS2, it's fairly straightforward!

- For x86_64 machine - Open the `MSYS2 UCRT64` and install this:
  
  ```bash
  pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-sdl3
  ```
- For ARM64 machine - Open the `MSYS2 CLANGARM64` and install this instead:
  
  ```bash
  pacman -S mingw-w64-clang-aarch64-gcc mingw-w64-clang-aarch64-ninja mingw-w64-clang-aarch64-cmake mingw-w64-clang-aarch64-sdl3
  ```

## Develop
- Clone this repo
- Build

  ```bash
  cmake -S . -B build
  cmake --build build
  ```
- Run

  I'll include some test ROMs in repo, so you can run the emulator along with those ROMs

  ```bash
  ./build/chip8 /path/to/ROM/
  ```
