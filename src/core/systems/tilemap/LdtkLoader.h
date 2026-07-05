#pragma once
#include "raylib.h"
#include <string>
#include <vector>

// ============================================================
//  LdtkLoader  —  reads a .ldtk project file and extracts
//  wall rectangles and entity spawn points per level.
//
//  Layer naming conventions (case-sensitive in LDtk):
//    "Walls"    — IntGrid layer; any non-zero value = solid
//    "Entities" — Entities layer; entity types used:
//                   "PlayerSpawn", "EnemySpawn"
// ============================================================

struct LdtkEntity
{
    std::string identifier; // e.g. "PlayerSpawn", "EnemySpawn"
    Vector2 position;       // World-space pixel position
};

// A single placed tile: where to draw it (dst) and which part of the
// tileset image to sample (src). src.width/height are negative when the
// tile is flipped, matching raylib's DrawTexturePro convention.
struct LdtkTile
{
    Rectangle src;
    Rectangle dst;
};

// One visual tile layer ("Tiles" / "AutoLayer"). tilesetRelPath is the path
// stored in the .ldtk (often outside the project); the renderer decides which
// texture to actually load.
struct LdtkTileLayer
{
    std::string tilesetRelPath;
    int gridSize = 16;
    std::vector<LdtkTile> tiles;
};

struct LdtkLevel
{
    std::string identifier;
    int worldX = 0;
    int worldY = 0;
    int pxWidth = 0;
    int pxHeight = 0;
    std::vector<Rectangle> walls;
    std::vector<LdtkEntity> entities;
    std::vector<LdtkTileLayer> tileLayers; // In LDtk order (topmost layer first)
};

class LdtkLoader
{
public:
    // Appends all levels found in filePath into outLevels.
    // Returns false if the file cannot be opened or parsed.
    static bool load(const std::string &filePath, std::vector<LdtkLevel> &outLevels);
};
