#pragma once
#include "raylib.h"
#include "core/entities/player/states/StateMachine.h"
#include "core/systems/combat/Hitbox.h"
#include "core/systems/combat/Hurtbox.h"
#include <vector>

// ============================================================
//  Player  —  top-down character controller.
//
//  Owns the StateMachine; states drive velocity each frame.
//  moveAndSlide() integrates velocity into position and keeps
//  the hurtbox rect in sync.
// ============================================================
class Player
{
public:
    // --- Transform ---
    Vector2 position = {0.0f, 0.0f};
    Vector2 velocity = {0.0f, 0.0f};
    Vector2 facing = {0.0f, 1.0f}; // Default: facing down

    // --- Stats ---
    float maxHealth = 100.0f;
    float currentHealth = 100.0f;

    // --- Collision ---
    float collisionRadius = 12.0f;

    // --- Combat ---
    Hitbox meleeHitbox;
    Hurtbox hurtbox;

    // --- Environment ---
    // Pointer to the active level's wall list; set by main after tilemap creation.
    const std::vector<Rectangle> *walls = nullptr;

    // --- FSM ---
    StateMachine fsm;

    void init();
    void update(float delta);
    void draw();

    // Integrates velocity -> position; keeps hurtbox in sync.
    // Call from states (mirrors Godot's move_and_slide).
    void moveAndSlide(float delta);

    void takeDamage(float amount, Vector2 sourcePos);

    // Axis-aligned bounding box around the player sprite
    Rectangle getBounds() const
    {
        return {
            position.x - collisionRadius,
            position.y - collisionRadius,
            collisionRadius * 2.0f,
            collisionRadius * 2.0f};
    }

private:
    void onDeath();
};
