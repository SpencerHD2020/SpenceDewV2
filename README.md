# SpenceDew

A top-down action RPG with deep combat and NPC romance systems. Built in C++ with raylib.

## Quick Start

**Prerequisites:** Visual Studio 2022 Community, CMake 4+, Git

```bash
# Configure (downloads dependencies — one-time, ~200 MB)
cmake -S . -B build -G "Visual Studio 17 2022"

# Build & run
cmake --build build --config Debug
./build/Debug/SpenceDew.exe
```

Or open this folder in VS Code and press **Ctrl+Shift+B**.

## Controls

| Key | Action |
|---|---|
| WASD / Arrows | Move |
| Left Shift | Sprint |
| Z / Left Mouse | Attack |
| Left Ctrl / X | Dodge (I-frames) |

## Dependencies (auto-fetched by CMake)

- [raylib 5.5](https://github.com/raysan5/raylib) — rendering, input, audio
- [EnTT 3.14](https://github.com/skypjack/entt) — ECS (used in later phases)
- [nlohmann/json 3.11](https://github.com/nlohmann/json) — save/load, dialogue data
