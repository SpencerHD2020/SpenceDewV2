#include "core/entities/NPC.h"

void NPC::moveAndSlide(float delta)
{
    // --- X axis ---
    position.x += velocity.x * delta;
    if (walls)
    {
        Rectangle bounds = getBounds();
        for (const Rectangle &wall : *walls)
        {
            if (CheckCollisionRecs(bounds, wall))
            {
                if (velocity.x > 0.0f)
                    position.x = wall.x - collisionRadius;
                else if (velocity.x < 0.0f)
                    position.x = wall.x + wall.width + collisionRadius;
                velocity.x = 0.0f;
                bounds = getBounds();
            }
        }
    }

    // --- Y axis ---
    position.y += velocity.y * delta;
    if (walls)
    {
        Rectangle bounds = getBounds();
        for (const Rectangle &wall : *walls)
        {
            if (CheckCollisionRecs(bounds, wall))
            {
                if (velocity.y > 0.0f)
                    position.y = wall.y - collisionRadius;
                else if (velocity.y < 0.0f)
                    position.y = wall.y + wall.height + collisionRadius;
                velocity.y = 0.0f;
                bounds = getBounds();
            }
        }
    }
}
