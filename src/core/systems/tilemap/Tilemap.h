#pragma once
#include "raylib.h"
#include "core/systems/tilemap/LdtkLoader.h"
#include <string>
#include <vector>

constexpr int TILE_SIZE = 32;

// ============================================================
//  Tilemap  —  placeholder hardcoded room for Phase 1.
//  Replace with LDtk JSON loader in Phase 2.
//
//  Coordinate system: world origin (0, 0) is the room centre.
//  Room is 24 × 18 tiles (768 × 576 px).
// ============================================================
struct Tilemap
{
    std::vector<Rectangle> walls;

    // --- Visual tiles loaded from an LDtk level ---
    Texture2D tileset = {};
    struct TileQuad
    {
        Rectangle src;
        Rectangle dst;
    };
    std::vector<TileQuad> tiles;
    bool showWallDebug = false; // Draw collision rects on top of the tiles

    // Load the tileset texture and flatten every visual tile layer of the level
    // into a single draw list. Call after InitWindow(). tilesetPath is a
    // project-relative path (the .ldtk's own relPath usually points outside the
    // project, so the caller supplies the shipped asset location).
    void loadVisuals(const LdtkLevel &level, const std::string &tilesetPath)
    {
        tileset = LoadTexture(tilesetPath.c_str());
        if (tileset.id == 0)
        {
            TraceLog(LOG_WARNING,
                     "[Tilemap] Tileset not found: %s — tiles will not render.",
                     tilesetPath.c_str());
        }
        // Draw bottom layer first: LDtk stores layers topmost-first, so reverse.
        for (auto it = level.tileLayers.rbegin(); it != level.tileLayers.rend(); ++it)
            for (const LdtkTile &t : it->tiles)
                tiles.push_back({t.src, t.dst});
    }

    void unloadVisuals()
    {
        if (tileset.id != 0)
        {
            UnloadTexture(tileset);
            tileset = {};
        }
        tiles.clear();
    }

    static Tilemap createTestRoom()
    {
        Tilemap tm;

        // Room outer boundary: 768 × 576 px, top-left at (-384, -288)
        constexpr float L = -384.0f, T = -288.0f;
        constexpr float W = 768.0f, H = 576.0f;
        constexpr float S = static_cast<float>(TILE_SIZE);

        // --- Outer walls (1 tile thick) ---
        tm.walls.push_back({L, T, W, S});         // top
        tm.walls.push_back({L, T + H - S, W, S}); // bottom
        tm.walls.push_back({L, T, S, H});         // left
        tm.walls.push_back({L + W - S, T, S, H}); // right

        // --- Interior pillars (2×2 tiles = 64×64 px) ---
        tm.walls.push_back({-192.0f, -160.0f, 64.0f, 64.0f}); // top-left
        tm.walls.push_back({128.0f, -160.0f, 64.0f, 64.0f});  // top-right
        tm.walls.push_back({-192.0f, 96.0f, 64.0f, 64.0f});   // bottom-left
        tm.walls.push_back({128.0f, 96.0f, 64.0f, 64.0f});    // bottom-right

        return tm;
    }

    void draw() const
    {
        // --- Visual tiles (if a tileset was loaded) ---
        if (tileset.id != 0)
        {
            for (const TileQuad &q : tiles)
                DrawTexturePro(tileset, q.src, q.dst, {0.0f, 0.0f}, 0.0f, WHITE);
        }

        // --- Collision debug (off by default once real tiles are present) ---
        if (showWallDebug || tileset.id == 0)
        {
            for (const Rectangle &wall : walls)
            {
                DrawRectangleRec(wall, DARKGRAY);
                DrawRectangleLinesEx(wall, 1.0f, BLACK);
            }
        }
    }
};
