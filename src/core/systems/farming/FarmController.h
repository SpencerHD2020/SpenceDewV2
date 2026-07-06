#pragma once
#include "raylib.h"
#include "core/systems/tilemap/LdtkLoader.h"
#include <unordered_map>
#include <cstdint>

// ============================================================
//  FarmController  —  minimal, stateful farm-tile handling.
//
//  TEMPORARY design (no inventory yet): one button cycles a
//  farmable tile through its states based on what it currently
//  is — the classic Stardew-style "do-it-all" tool stand-in:
//
//      Soil  --till-->  Tilled  --plant-->  Planted  --water-->  Watered
//
//  Which tiles are farmable is decided at build() time from the
//  level's tile data: a cell is farmable if it has a tile, is not
//  a wall, and its tile id is NOT in the excluded list (currently
//  { 3, 16 }). That list is expected to grow / be replaced later.
//
//  Growth cycles, crop types, and per-seed data are intentionally
//  out of scope for now — FarmState is a flat enum so it can be
//  extended into a proper crop/growth model later without churn.
// ============================================================

enum class FarmState
{
    Soil,    // Farmable ground, not yet tilled
    Tilled,  // Hoed, ready to plant
    Planted, // Seed in the ground
    Watered  // Planted + watered (end of the temporary cycle)
};

class FarmController
{
public:
    // Excluded tile ids: farmable cells must NOT contain any of these.
    // (Grows / gets replaced later — see the header comment.)
    static bool isExcludedTileId(int id) { return id == 3 || id == 16; }

    // Rebuild the farmable-cell set from a level's tiles + walls.
    void build(const LdtkLevel &level);
    void clear();

    // Advance the tile containing `world`. Returns true if state changed.
    bool interact(Vector2 world);

    bool isFarmable(Vector2 world) const;

    // Grid-aligned cell rect containing `world` (regardless of farmable).
    Rectangle cellRect(Vector2 world) const;

    int gridSize() const { return m_gridSize; }

    void draw() const;

private:
    struct Cell
    {
        Rectangle rect;
        FarmState state = FarmState::Soil;
    };

    static uint64_t key(int col, int row)
    {
        return (static_cast<uint64_t>(static_cast<uint32_t>(col)) << 32) |
               static_cast<uint32_t>(row);
    }
    void colRow(Vector2 world, int &col, int &row) const;

    int m_gridSize = 16;
    int m_worldX = 0;
    int m_worldY = 0;
    std::unordered_map<uint64_t, Cell> m_cells; // Farmable cells only
};
