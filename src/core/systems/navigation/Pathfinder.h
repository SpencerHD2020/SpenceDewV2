#pragma once
#include "raylib.h"
#include <vector>

// ============================================================
//  Pathfinder  —  grid-based A* for enemy navigation.
//
//  Usage (call once when tilemap changes):
//    Pathfinder pf;
//    pf.buildGrid(tilemap.walls, {worldOriginX, worldOriginY, worldW, worldH});
//
//  Per enemy chase cycle:
//    std::vector<Vector2> wp = pf.findPath(enemy.position, player.position);
//    // Walk toward wp[0], advance when reached, etc.
// ============================================================
class Pathfinder
{
public:
    // Rebuild the nav-grid from a fresh wall list.
    // worldBounds: {x, y, width, height} of the navigable area.
    void buildGrid(const std::vector<Rectangle> &walls, Rectangle worldBounds);

    // Returns a list of world-space waypoints from 'from' to 'to'.
    // Returns empty vector if no path exists or grid is not built.
    std::vector<Vector2> findPath(Vector2 from, Vector2 to) const;

    bool isBuilt() const { return !m_solid.empty(); }

private:
    // Flat 2D grid stored row-major [row * m_cols + col]
    std::vector<bool> m_solid;
    int m_cols = 0;
    int m_rows = 0;
    float m_originX = 0.0f;
    float m_originY = 0.0f;
    int m_cell = 32; // Cell size in pixels (matches TILE_SIZE)

    // Convert world pos to grid cell (clamped to grid bounds)
    void worldToGrid(Vector2 pos, int &outCol, int &outRow) const;
    Vector2 gridToWorld(int col, int row) const;
    bool isValid(int col, int row) const;
    bool isSolid(int col, int row) const;
    int heuristic(int c1, int r1, int c2, int r2) const;
};
