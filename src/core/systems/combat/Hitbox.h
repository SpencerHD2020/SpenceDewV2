#pragma once
#include "raylib.h"
#include "Hurtbox.h"
#include <vector>

// ============================================================
//  Hitbox  —  deals damage when it overlaps a Hurtbox.
//  Call checkOverlap() every frame while the hitbox is active
//  (e.g., during the attack window in StateAttack).
// ============================================================
struct Hitbox {
    Rectangle rect          = {};       // World-space AABB, positioned by the owning state
    float     damage        = 10.0f;
    float     knockbackForce = 200.0f;
    bool      isActive      = false;
    void*     ownerEntity   = nullptr;  // Set to owner to prevent self-damage

    // Pass a list of all live hurtboxes in the scene.
    // Typically called from the game's collision-resolution step.
    void checkOverlap(const std::vector<Hurtbox*>& hurtboxes) const {
        if (!isActive) return;
        Vector2 center = {rect.x + rect.width * 0.5f, rect.y + rect.height * 0.5f};
        for (Hurtbox* hb : hurtboxes) {
            if (!hb) continue;
            if (ownerEntity && ownerEntity == hb->ownerEntity) continue;
            if (CheckCollisionRecs(rect, hb->rect)) {
                hb->takeDamage(damage, knockbackForce, center);
            }
        }
    }
};
