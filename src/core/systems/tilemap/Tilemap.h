#pragma once
#include "raylib.h"
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
        for (const Rectangle &wall : walls)
        {
            DrawRectangleRec(wall, DARKGRAY);
            DrawRectangleLinesEx(wall, 1.0f, BLACK);
        }
    }
};
