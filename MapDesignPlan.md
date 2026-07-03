# Map Design Plan

## Tools
- **[LDtk](https://ldtk.io)** — level editor; paint tiles, place entities, export to JSON

## Free Tilesets
- **[Kenney.nl](https://kenney.nl/assets)** — top-down RPG tiles (grass, paths, walls, buildings); free, no attribution required

## Workflow
1. Download a Kenney tileset PNG and drop it in `assets/sprites/`
2. Open LDtk, import the tileset, paint your map
3. Export the `.ldtk` file to `assets/levels/`
4. Phase 2 adds the C++ loader that reads the JSON and renders it in-game

## Division of Work
| Task | Where |
|---|---|
| Painting tiles, designing rooms | LDtk |
| Game logic (doors, triggers, spawns) | C++ (ask Copilot) |
| Loading & rendering the LDtk file | C++ (ask Copilot when ready) |
