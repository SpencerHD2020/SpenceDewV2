#include "core/systems/navigation/Pathfinder.h"
#include <queue>
#include <vector>
#include <cmath>
#include <algorithm>
#include <climits>

// -------------------------------------------------------
// Grid build
// -------------------------------------------------------
void Pathfinder::buildGrid(const std::vector<Rectangle> &walls, Rectangle worldBounds)
{
    m_originX = worldBounds.x;
    m_originY = worldBounds.y;
    m_cols = static_cast<int>(worldBounds.width / m_cell) + 1;
    m_rows = static_cast<int>(worldBounds.height / m_cell) + 1;

    m_solid.assign(m_cols * m_rows, false);

    for (const Rectangle &wall : walls)
    {
        // Mark every cell overlapping this wall as solid
        int c0 = static_cast<int>((wall.x - m_originX) / m_cell);
        int r0 = static_cast<int>((wall.y - m_originY) / m_cell);
        int c1 = static_cast<int>((wall.x + wall.width - m_originX - 1.0f) / m_cell);
        int r1 = static_cast<int>((wall.y + wall.height - m_originY - 1.0f) / m_cell);

        for (int r = r0; r <= r1; ++r)
        {
            for (int c = c0; c <= c1; ++c)
            {
                if (isValid(c, r))
                    m_solid[r * m_cols + c] = true;
            }
        }
    }
}

// -------------------------------------------------------
// Helpers
// -------------------------------------------------------
void Pathfinder::worldToGrid(Vector2 pos, int &outCol, int &outRow) const
{
    outCol = static_cast<int>((pos.x - m_originX) / m_cell);
    outRow = static_cast<int>((pos.y - m_originY) / m_cell);
    outCol = std::max(0, std::min(outCol, m_cols - 1));
    outRow = std::max(0, std::min(outRow, m_rows - 1));
}

Vector2 Pathfinder::gridToWorld(int col, int row) const
{
    return {
        m_originX + (col + 0.5f) * m_cell,
        m_originY + (row + 0.5f) * m_cell};
}

bool Pathfinder::isValid(int col, int row) const
{
    return col >= 0 && col < m_cols && row >= 0 && row < m_rows;
}

bool Pathfinder::isSolid(int col, int row) const
{
    if (!isValid(col, row))
        return true;
    return m_solid[row * m_cols + col];
}

int Pathfinder::heuristic(int c1, int r1, int c2, int r2) const
{
    // Octile distance scaled by 10 (diagonal cost ≈ 14)
    int dc = std::abs(c1 - c2);
    int dr = std::abs(r1 - r2);
    return 10 * (dc + dr) - 4 * std::min(dc, dr);
}

// -------------------------------------------------------
// A* search
// -------------------------------------------------------
std::vector<Vector2> Pathfinder::findPath(Vector2 from, Vector2 to) const
{
    if (m_solid.empty())
        return {};

    int sc, sr, ec, er;
    worldToGrid(from, sc, sr);
    worldToGrid(to, ec, er);

    if (sc == ec && sr == er)
        return {};
    if (isSolid(ec, er))
        return {}; // Goal is inside a wall

    const int N = m_cols * m_rows;

    std::vector<int> gCost(N, INT_MAX);
    std::vector<int> parent(N, -1);
    std::vector<bool> closed(N, false);

    // Min-heap: {fCost, index}
    using Node = std::pair<int, int>;
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;

    auto idx = [&](int c, int r)
    { return r * m_cols + c; };

    int startIdx = idx(sc, sr);
    gCost[startIdx] = 0;
    open.push({heuristic(sc, sr, ec, er), startIdx});

    // 8-directional neighbours
    const int dc[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    const int dr[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    const int cost[] = {14, 10, 14, 10, 10, 14, 10, 14};

    while (!open.empty())
    {
        auto [f, curIdx] = open.top();
        open.pop();

        if (closed[curIdx])
            continue;
        closed[curIdx] = true;

        int cc = curIdx % m_cols;
        int cr = curIdx / m_cols;

        if (cc == ec && cr == er)
        {
            // Retrace path
            std::vector<Vector2> path;
            int i = curIdx;
            while (i != startIdx)
            {
                int c = i % m_cols;
                int r = i / m_cols;
                path.push_back(gridToWorld(c, r));
                i = parent[i];
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        for (int d = 0; d < 8; ++d)
        {
            int nc = cc + dc[d];
            int nr = cr + dr[d];
            if (!isValid(nc, nr) || isSolid(nc, nr))
                continue;

            // Prevent corner-cutting
            if (d == 0 && (isSolid(cc - 1, cr) || isSolid(cc, cr - 1)))
                continue;
            if (d == 2 && (isSolid(cc + 1, cr) || isSolid(cc, cr - 1)))
                continue;
            if (d == 5 && (isSolid(cc - 1, cr) || isSolid(cc, cr + 1)))
                continue;
            if (d == 7 && (isSolid(cc + 1, cr) || isSolid(cc, cr + 1)))
                continue;

            int ni = idx(nc, nr);
            if (closed[ni])
                continue;

            int ng = gCost[curIdx] + cost[d];
            if (ng < gCost[ni])
            {
                gCost[ni] = ng;
                parent[ni] = curIdx;
                int h = heuristic(nc, nr, ec, er);
                open.push({ng + h, ni});
            }
        }
    }

    return {}; // No path found
}
