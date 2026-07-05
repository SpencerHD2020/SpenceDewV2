#pragma once
#include "raylib.h"
#include <functional>

// ============================================================
//  Hurtbox  —  receives damage when overlapped by a Hitbox.
//  Attach one to any entity that can take damage.
// ============================================================
struct Hurtbox
{
    Rectangle rect = {}; // World-space AABB, updated by owner each frame
    bool isInvulnerable = false;
    void *ownerEntity = nullptr; // Raw pointer used only to prevent self-damage

    // Called by Hitbox::checkOverlap when a hit lands.
    // Signature: (float damage, float knockbackForce, Vector2 sourcePosition)
    std::function<void(float, float, Vector2)> onDamageReceived;

    void takeDamage(float amount, float knockbackForce, Vector2 sourcePos)
    {
        if (isInvulnerable)
            return;
        if (onDamageReceived)
            onDamageReceived(amount, knockbackForce, sourcePos);
    }

    void setInvulnerable(bool value) { isInvulnerable = value; }
};
