#pragma once
#include "raylib.h"
#include "Hurtbox.h"
#include <vector>

// ============================================================
//  Projectile  —  a moving entity with a hitbox.
//
//  Deactivates on:  wall hit | hurtbox hit | lifetime expiry.
//  Owned by a scene-level list in main.cpp.
//  Dead projectiles (active == false) should be erased
//  at the end of the frame.
//
//  Usage:
//    Projectile bolt;
//    bolt.init(player.position, player.facing, 400.0f,
//              20.0f, 200.0f, &player);
//    projectiles.push_back(bolt);
// ============================================================
struct Projectile
{
    Vector2 position = {};
    Vector2 velocity = {};
    float radius = 7.0f;
    float damage = 20.0f;
    float knockback = 200.0f;
    float lifetime = 2.5f;
    bool active = true;
    Color color = {160, 60, 220, 255}; // Purple magic bolt
    void *owner = nullptr;             // Prevents hitting its own owner's hurtbox

    void init(Vector2 pos, Vector2 dir, float speed,
              float dmg, float kb, void *ownerEntity)
    {
        position = pos;
        velocity = {dir.x * speed, dir.y * speed};
        damage = dmg;
        knockback = kb;
        owner = ownerEntity;
        active = true;
    }

    void update(float delta,
                const std::vector<Rectangle> &walls,
                const std::vector<Hurtbox *> &hurtboxes)
    {
        if (!active)
            return;

        lifetime -= delta;
        if (lifetime <= 0.0f)
        {
            active = false;
            return;
        }

        position.x += velocity.x * delta;
        position.y += velocity.y * delta;

        // --- Wall collision ---
        for (const Rectangle &wall : walls)
        {
            if (CheckCollisionCircleRec(position, radius, wall))
            {
                active = false;
                return;
            }
        }

        // --- Hurtbox collision ---
        Rectangle bounds = getBounds();
        for (Hurtbox *hb : hurtboxes)
        {
            if (!hb || hb->isInvulnerable)
                continue;
            if (owner && owner == hb->ownerEntity)
                continue; // No self-damage
            if (CheckCollisionRecs(bounds, hb->rect))
            {
                hb->takeDamage(damage, knockback, position);
                active = false;
                return;
            }
        }
    }

    void draw() const
    {
        if (!active)
            return;
        // Glow trail
        DrawCircleV(position, radius + 4.0f, {color.r, color.g, color.b, 50});
        DrawCircleV(position, radius + 2.0f, {color.r, color.g, color.b, 120});
        DrawCircleV(position, radius, color);
        DrawCircleV(position, radius * 0.4f, WHITE);
    }

    Rectangle getBounds() const
    {
        return {position.x - radius, position.y - radius,
                radius * 2.0f, radius * 2.0f};
    }
};
