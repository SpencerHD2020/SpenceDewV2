#include "core/entities/friends/Friend.h"
#include "raymath.h"

void Friend::init(Vector2 spawnPos,
                  const std::string &id,
                  const std::string &name,
                  const std::string &dialogue)
{
    position = spawnPos;
    npcId = id;
    displayName = name;
    dialogueId = dialogue;
}

bool Friend::playerInRange(Vector2 playerPos) const
{
    return Vector2Distance(playerPos, position) <= interactionRadius;
}

void Friend::update(float delta)
{
    // Static for now. Advance the sprite clock if a real sheet is loaded so
    // idle animations play; movement/wander can be added here later.
    if (sprite.hasTexture())
        sprite.update(delta);
}

void Friend::draw() const
{
    if (sprite.hasTexture())
    {
        sprite.draw(position, 1.0f);
    }
    else
    {
        // Placeholder body until art is wired up.
        DrawCircleV(position, collisionRadius, debugColor);
        Vector2 f = {
            position.x + facing.x * (collisionRadius + 6.0f),
            position.y + facing.y * (collisionRadius + 6.0f)};
        DrawLineV(position, f, DARKBLUE);
    }

    // Name tag above the head.
    if (!displayName.empty())
    {
        int tw = MeasureText(displayName.c_str(), 10);
        DrawText(displayName.c_str(),
                 static_cast<int>(position.x - tw * 0.5f),
                 static_cast<int>(position.y - collisionRadius - 14.0f),
                 10, RAYWHITE);
    }
}
