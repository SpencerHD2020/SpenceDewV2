#include "core/systems/farming/FarmController.h"
#include <cmath>
#include <unordered_set>
#include <unordered_map>

void FarmController::clear()
{
    m_cells.clear();
}

void FarmController::colRow(Vector2 world, int &col, int &row) const
{
    col = static_cast<int>(std::floor((world.x - m_worldX) / static_cast<float>(m_gridSize)));
    row = static_cast<int>(std::floor((world.y - m_worldY) / static_cast<float>(m_gridSize)));
}

void FarmController::build(const LdtkLevel &level)
{
    m_cells.clear();
    m_worldX = level.worldX;
    m_worldY = level.worldY;
    m_gridSize = level.tileLayers.empty() ? 16 : level.tileLayers.front().gridSize;
    const float gs = static_cast<float>(m_gridSize);

    // Wall cells (excluded from farming).
    std::unordered_set<uint64_t> wallCells;
    for (const Rectangle &w : level.walls)
    {
        int col = static_cast<int>(std::floor((w.x - m_worldX) / gs));
        int row = static_cast<int>(std::floor((w.y - m_worldY) / gs));
        wallCells.insert(key(col, row));
    }

    // Per-cell: does it have any tile, and does it contain an excluded tile id?
    struct CellInfo
    {
        bool hasTile = false;
        bool excluded = false;
    };
    std::unordered_map<uint64_t, CellInfo> info;

    for (const LdtkTileLayer &layer : level.tileLayers)
    {
        for (const LdtkTile &t : layer.tiles)
        {
            int col = static_cast<int>(std::floor((t.dst.x - m_worldX) / gs));
            int row = static_cast<int>(std::floor((t.dst.y - m_worldY) / gs));
            CellInfo &ci = info[key(col, row)];
            ci.hasTile = true;
            if (isExcludedTileId(t.tileId))
                ci.excluded = true;
        }
    }

    for (const auto &[k, ci] : info)
    {
        if (!ci.hasTile || ci.excluded || wallCells.count(k))
            continue;
        int col = static_cast<int>(k >> 32);
        int row = static_cast<int>(k & 0xffffffff);
        Cell cell;
        cell.rect = {m_worldX + col * gs, m_worldY + row * gs, gs, gs};
        cell.state = FarmState::Soil;
        m_cells[k] = cell;
    }
}

bool FarmController::isFarmable(Vector2 world) const
{
    int col, row;
    colRow(world, col, row);
    return m_cells.count(key(col, row)) != 0;
}

Rectangle FarmController::cellRect(Vector2 world) const
{
    int col, row;
    colRow(world, col, row);
    const float gs = static_cast<float>(m_gridSize);
    return {m_worldX + col * gs, m_worldY + row * gs, gs, gs};
}

bool FarmController::interact(Vector2 world)
{
    int col, row;
    colRow(world, col, row);
    auto it = m_cells.find(key(col, row));
    if (it == m_cells.end())
        return false;

    switch (it->second.state)
    {
    case FarmState::Soil:
        it->second.state = FarmState::Tilled;
        return true;
    case FarmState::Tilled:
        it->second.state = FarmState::Planted;
        return true;
    case FarmState::Planted:
        it->second.state = FarmState::Watered;
        return true;
    case FarmState::Watered:
    default:
        return false; // End of the temporary cycle (growth handled later).
    }
}

static void drawSprout(const Rectangle &r, Color c)
{
    float cx = r.x + r.width * 0.5f;
    float cy = r.y + r.height * 0.6f;
    DrawLineEx({cx, cy}, {cx, cy - r.height * 0.25f}, 1.5f, c);
    DrawCircle(static_cast<int>(cx), static_cast<int>(cy - r.height * 0.25f), 2.0f, c);
}

void FarmController::draw() const
{
    for (const auto &[k, cell] : m_cells)
    {
        const Rectangle &r = cell.rect;
        switch (cell.state)
        {
        case FarmState::Soil:
            // Faint outline so testers can see where farming is allowed.
            DrawRectangleLinesEx(r, 1.0f, Fade(GREEN, 0.30f));
            break;
        case FarmState::Tilled:
            DrawRectangleRec(r, Fade(BROWN, 0.75f));
            DrawRectangleLinesEx(r, 1.0f, Fade(DARKBROWN, 0.9f));
            break;
        case FarmState::Planted:
            DrawRectangleRec(r, Fade(BROWN, 0.75f));
            drawSprout(r, GREEN);
            break;
        case FarmState::Watered:
            DrawRectangleRec(r, Fade(DARKBROWN, 0.9f));
            DrawRectangleLinesEx(r, 1.0f, Fade(SKYBLUE, 0.6f));
            drawSprout(r, LIME);
            break;
        }
    }
}
