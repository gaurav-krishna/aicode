# LuaJIT C++ Wrapper (Embedded)

A small C++ wrapper around LuaJIT for embedding Lua scripts with JIT controls, error handling, and simple value passing.

## Build

Requirements:
- CMake >= 3.16
- C++17 compiler
- LuaJIT development files and `pkg-config` available on the system

On Debian/Ubuntu:

```bash
sudo apt-get update && sudo apt-get install -y build-essential cmake pkg-config libluajit-5.1-dev
```

Configure and build:

```bash
cmake -S . -B build
cmake --build build -j
```

Run the example:

```bash
./build/example_embed
```

## Usage

See `examples/main.cpp` and `include/LuaJITWrapper.hpp`.

- Create `LuaJITContext`, optionally pass JIT options
- Register C++ functions via `registerFunction`
- Set globals and run scripts, call Lua global functions
- Use `disableOsUnsafe` for a basic sandbox (removes `os.execute`, etc.)
