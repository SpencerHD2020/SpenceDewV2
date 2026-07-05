#include "core/entities/friends/PotentialPartner.h"

void PotentialPartner::init(Vector2 spawnPos,
                            const std::string &id,
                            const std::string &name,
                            const std::string &dialogue)
{
    Friend::init(spawnPos, id, name, dialogue);
    debugColor = PINK; // Distinguish romanceable NPCs from plain friends
}

void PotentialPartner::draw() const
{
    Friend::draw();

    // Small heart marker above the name tag.
    float hx = position.x;
    float hy = position.y - collisionRadius - 24.0f;
    DrawCircle(static_cast<int>(hx - 3.0f), static_cast<int>(hy), 3.0f, RED);
    DrawCircle(static_cast<int>(hx + 3.0f), static_cast<int>(hy), 3.0f, RED);
    DrawTriangle(
        {hx - 6.0f, hy + 1.0f},
        {hx + 6.0f, hy + 1.0f},
        {hx, hy + 7.0f},
        RED);
}
