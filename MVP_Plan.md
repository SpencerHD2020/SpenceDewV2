# SpenceDew Game Design & Development Plan
## Tech Stack: C++ + raylib

---

## 1. Tech Stack

| Layer | Technology | Why |
|---|---|---|
| **Rendering / Input / Audio** | **raylib 5.5** | Single-dependency 2D rendering library, pure C API, massive community, excellent AI training coverage. Zero external IDE required — write and build entirely in VS Code. |
| **Entity / Component System** | **EnTT 3.14** (header-only) | When the entity count grows (NPCs, enemies, items), EnTT provides a fast, type-safe ECS. Not required in early MVP — plain C++ classes are fine until you feel the pain. |
| **Save / Load / Dialogue Data** | **nlohmann/json 3.11** (header-only) | Dead-simple JSON in C++. Used for save files, NPC relationship data, and loading Ink-compiled dialogue trees. |
| **Level Design** | **LDtk** (free editor) | Modern tilemap + entity placement editor. Exports to JSON; a simple C++ loader reads it. Keeps level data out of code. |
| **Dialogue & Narrative** | **Ink** (free) | Industry-standard branching dialogue scripting language. Compiles to JSON; loaded at runtime by a small C++ reader. |
| **Build System** | **CMake 4.x** | All dependencies are fetched and built automatically via `FetchContent` — just configure and build. |
| **Version Control** | **Git + GitHub** | Essential checkpoints when AI generates chunks of code that may need reverting. |

**No external IDE.** Everything — editing, building, debugging — happens in VS Code via the CMake Tools extension.

---

## 2. Architecture Overview

The architecture directly mirrors the Godot version but expressed in idiomatic C++:

### Finite State Machine (FSM)
`State` → `StateMachine` → `Player / Enemy`

- Each behaviour is a class that extends `State` and overrides `enter()`, `exit()`, `physicsUpdate()`.
- `StateMachine` owns all states as `unique_ptr`s, drives the active one each frame.
- Transitioning is a single call: `stateMachine->changeState("run")`.
- No macros, no reflection — just plain virtual dispatch.

### Hitbox / Hurtbox Combat
Mirrors the Godot Area2D pattern using AABB (`Rectangle`) overlap tests:

- `Hitbox` — carries damage and an active flag. Checks overlap against a list of `Hurtbox*` each frame.
- `Hurtbox` — carries an `onDamageReceived` callback (like Godot signals). The owner entity wires up its own damage logic.
- Self-damage prevented via matching `ownerEntity` void pointers.

### Event / Signal Equivalent
Godot's signals become `std::function` callbacks stored on the relevant struct (see `Hurtbox::onDamageReceived`). For global events (enemy death → XP update), a lightweight event bus or a simple global manager will be introduced in Phase 3.

### NPC Relationship Manager
A singleton class (static instance, accessed globally) holding a `std::unordered_map<std::string, NpcData>` per NPC. `NpcData` holds affection level, dialogue progress index, gifts-given flags. Serialised/deserialised with nlohmann/json.

---

## 3. Project Structure

```
SpenceDewV2/
├── CMakeLists.txt               ← Single build file; handles all dependency fetching
├── assets/
│   ├── sprites/                 ← Sprite sheets (.png)
│   └── levels/                  ← LDtk .ldtk project files
├── src/
│   ├── main.cpp                 ← Window init, game loop, HUD
│   └── core/
│       ├── Input.h              ← Centralised key/button mappings
│       ├── entities/
│       │   ├── player/
│       │   │   ├── Player.h / .cpp
│       │   │   └── states/
│       │   │       ├── State.h
│       │   │       ├── StateMachine.h / .cpp
│       │   │       ├── StateIdle.h
│       │   │       ├── StateRun.h
│       │   │       ├── StateAttack.h
│       │   │       └── StateDodge.h
│       │   └── enemies/         ← (Phase 2)
│       └── systems/
│           ├── combat/
│           │   ├── Hitbox.h
│           │   └── Hurtbox.h
│           ├── dialogue/        ← (Phase 3) Ink JSON reader
│           └── relationships/   ← (Phase 3) NPC manager
└── .vscode/
    ├── settings.json            ← CMake Tools configuration
    ├── tasks.json               ← Build Debug / Release tasks
    ├── launch.json              ← MSVC debugger launch config
    └── extensions.json          ← Recommended extensions
```

---

## 4. Build Instructions

**Prerequisites (one-time):**
1. Visual Studio 2022 Community (MSVC compiler) — already installed
2. CMake 4.x — installed via winget
3. Git — already installed

**First build** (downloads all dependencies ~200 MB; only done once):
```
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Debug
```

Or use **Ctrl+Shift+B** in VS Code (runs the default "Build Debug" task).

The executable lands at `build/Debug/SpenceDew.exe`.

---

## 5. MVP Schedule

### Phase 1: Foundation (Weeks 1–2) ✅ Complete
- [x] Project structure & build system (CMake + FetchContent)
- [x] Player controller — 8-way movement, walk/sprint
- [x] FSM — Idle, Run, Attack, Dodge states
- [x] Hitbox / Hurtbox collision system
- [x] Camera follow (2D Camera2D in raylib)
- [x] Placeholder tilemap (hardcoded rectangles as walls)

### Phase 2: Deep Combat Prototyping (Weeks 3–5)
- [x] Spritesheet loader + animation system (`AnimatedSprite` helper class)
- [ ] LDtk level loader (tilemap + collision layers from JSON) ← **code done; requires manual level design (see below)**
- [x] Melee combat: hitbox timing, knockback
- [x] 1 Magic attack: projectile entity (KEY_C to cast)
- [x] Basic Enemy: Chase + Attack + Die FSM
- [x] Simple enemy AI pathfinding (grid-based A*)
- [x] Health bar HUD, damage numbers

#### ⚠️ Manual Steps Required for Phase 2

The following items cannot be automated — you must do them yourself:

**1. Sprite Sheet Artwork**
Create or source sprite sheets for the player character and enemy and place them in `assets/sprites/`. The `AnimatedSprite` system expects a grid-layout PNG (all frames the same size) paired with a `.json` description file. The recommended tool is [Aseprite](https://www.aseprite.org/) (paid) or free alternatives like LibreSprite.

Expected JSON format alongside each PNG (e.g., `assets/sprites/player.png.json`):
```json
{
  "frameWidth": 32,
  "frameHeight": 32,
  "animations": {
    "idle_down":   { "row": 0, "frameCount": 4, "fps": 8,  "loop": true },
    "run_down":    { "row": 1, "frameCount": 6, "fps": 12, "loop": true },
    "run_left":    { "row": 2, "frameCount": 6, "fps": 12, "loop": true },
    "attack_down": { "row": 3, "frameCount": 4, "fps": 16, "loop": false },
    "cast_down":   { "row": 4, "frameCount": 3, "fps": 14, "loop": false },
    "dodge":       { "row": 5, "frameCount": 4, "fps": 20, "loop": false }
  }
}
```
> Right-facing animations are auto-generated by mirroring left-facing ones (set `flipX = true` on the sprite before playing the anim).

Until you have real assets, the game runs with placeholder coloured circles — fully playable.

**2. LDtk Level Design**
The `LdtkLoader` C++ code is complete, but it needs a `.ldtk` file to load. Steps:
1. Download [LDtk](https://ldtk.io/) (free, cross-platform).
2. Create a new project, set the default grid size to **32 px**.
3. Add an **IntGrid layer** named exactly `Walls` — value 1 = solid tile.
4. Add an **Entities layer** named exactly `Entities` and define two entity types:
   - `PlayerSpawn` (size 32×32)
   - `EnemySpawn` (size 32×32)
5. Design at least one level and place one `PlayerSpawn` and a few `EnemySpawn` markers.
6. Save the project as `assets/levels/level_01.ldtk`.
7. In `src/main.cpp` switch the `USE_LDTK` flag from `false` to `true`.

Until the file exists the game falls back to the hardcoded placeholder room.

**3. Animation Wiring (post-assets)**

This is a three-part job done entirely in code once you have the PNG files ready. Nothing here requires tools — just editing two source files.

**Part A — Load the sprite in `Player::init()` (Player.cpp)**

Add a public `AnimatedSprite sprite;` field to `Player.h` (include `core/animation/AnimatedSprite.h`), then at the bottom of `Player::init()`:

```cpp
sprite.load("assets/sprites/player.png"); // reads player.png.json automatically
```

**Part B — Drive the animation from each state**

Inside each state's `enter()` method, call `player->sprite.play(animName)`. The animation name must match a key in your JSON. Use `player->facing` to pick the right directional variant:

```cpp
// Helper lambda you can paste into each state header:
auto facingAnim = [&](const char* base) -> std::string {
    if (player->facing.y < -0.5f) return std::string(base) + "_up";
    if (player->facing.y >  0.5f) return std::string(base) + "_down";
    // Left/right: use the "left" row and flip the sprite
    player->sprite.flipX = (player->facing.x > 0.0f);
    return std::string(base) + "_left";
};
```

Then in each state's `enter()`:
```cpp
// StateIdle::enter()
player->sprite.play(facingAnim("idle"));

// StateRun::enter() — also update facing each frame in physicsUpdate
player->sprite.play(facingAnim("run"));

// StateAttack::enter()
player->sprite.play(facingAnim("attack"));

// StateCast::enter()
player->sprite.play(facingAnim("cast"));

// StateDodge::enter()
player->sprite.play("dodge"); // dodge is usually direction-agnostic
```

**Part C — Update and draw**

In `Player::update()`, add `sprite.update(delta);` **before** the FSM calls (so the frame timer advances every frame).

In `Player::draw()`, replace the `DrawCircleV` placeholder with:
```cpp
sprite.draw(position, 1.0f); // scale 1 = native pixel size
```
Leave the debug hitbox/hurtbox overlay lines in place — they're useful while tuning combat feel.

Do the exact same three steps for `Enemy` using `Enemy.h` / `Enemy.cpp`, with a separate `assets/sprites/enemy.png` and simpler animation names (`"idle"`, `"chase"`, `"attack"`, `"die"`).

> **Tip:** While developing art, it is fine to start with a single 32×32 coloured rectangle as each frame — Aseprite can create a file this simple in under a minute. Get the animation names and JSON wired up correctly first, then replace the art later.

### Phase 3: Social & Romance Core (Weeks 6–8)
- [ ] Ink dialogue JSON loader + branching dialogue runner
- [ ] Dialogue UI (textbox, name label, choice buttons)
- [ ] NPC entity with interaction trigger
- [ ] `RelationshipManager` singleton: affection score, gift flags, dialogue progression
- [ ] 1 Romanceable NPC with 3 affection tiers of unique dialogue

### Phase 4: Tying the Loop & Polish (Weeks 9–10)
- [ ] Mini-quest: NPC → combat zone → item retrieval → NPC reward
- [ ] Inventory: slot-based weapon/armour equip
- [ ] Basic save/load (nlohmann/json → `.sav` file)
- [ ] Audio: footsteps, attack SFX, ambient music (raylib `PlayMusicStream`)

---

## 6. New Challenges vs. Godot

These are things Godot handled automatically that now require explicit implementation:

| Challenge | Godot Had | raylib Approach |
|---|---|---|
| **Tilemap rendering & collision** | Built-in TileMap node | Load LDtk JSON; draw source rects from tileset texture; AABB collision vs. solid layer rects |
| **Animation system** | AnimationPlayer node | `AnimatedSprite` helper: store frame rects + durations, advance timer each frame |
| **Physics / collision resolution** | CharacterBody2D + `move_and_slide()` | Manual AABB sweep; `moveAndSlide()` in Player is currently position += velocity * dt — add environment collision in Phase 2 |
| **Scene / object lifecycle** | Node tree with `queue_free()` | Manual lists of active entities; mark-and-sweep or index-based removal |
| **Signals (event bus)** | Built-in `Signal` | `std::function` callbacks on structs; or a simple templated `EventBus<T>` class for global events |
| **Dialogue plugin** | Dialogic (drag-and-drop) | Write a 100–150 line Ink JSON reader; Ink's runtime is simple enough to drive from C++ |
| **Save / load** | Built-in serialisation | nlohmann/json; define `to_json` / `from_json` for save-state structs |
| **Pathfinding** | NavMesh (built-in) | Implement grid A* for enemies; navmesh is overkill for the MVP scope |

**Architectural risk:** Because there is no enforced scene graph, AI-generated code may introduce tight coupling between systems (e.g., an enemy directly accessing a UI element). Establish and hold to the rule: **entities never touch UI; UI reads from data, not from entities.**

---

## 7. AI Prompt Strategy

- **One system at a time.** "Write an `AnimatedSprite` class for raylib that plays frame sequences by name, given a spritesheet texture and a JSON frame-data file."
- **Specify the pattern.** "Using the existing `Hitbox` / `Hurtbox` pattern in this project, write a `Projectile` class for a magic bolt that checks for enemy hurtboxes and destroys itself on hit."
- **Give context.** Paste the relevant header file(s) so the AI generates code that fits the existing API rather than inventing new abstractions.
- **Review architecture, not just logic.** As a C++ professional you're well-placed to catch when AI introduces hidden globals, raw owning pointers, or circular includes.
