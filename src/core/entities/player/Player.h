#pragma once
#include "raylib.h"
#include "core/entities/player/states/StateMachine.h"
#include "core/systems/combat/Hitbox.h"
#include "core/systems/combat/Hurtbox.h"
#include "core/animation/AnimatedSprite.h"
#include <vector>
#include <functional>

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

    // --- Knockback ---
    Vector2 knockbackVelocity = {};
    float knockbackDecay = 900.0f; // px/s² deceleration

    // --- I-frames after taking damage ---
    float iFrameTimer = 0.0f;
    float iFrameDuration = 0.5f;

    // --- Environment ---
    const std::vector<Rectangle> *walls = nullptr;

    // --- Sprite ---
    AnimatedSprite sprite;

    // --- FSM ---
    StateMachine fsm;

    // --- Callbacks (set by main.cpp after init) ---
    // Called when visual damage feedback is needed (spawn a damage number).
    std::function<void(Vector2, float)> onDamageVisual;
    // Called when the player fires a magic projectile.
    std::function<void(Vector2, Vector2)> onFireProjectile; // (position, direction)

    void init();
    void update(float delta);
    void draw();

    void moveAndSlide(float delta);
    void takeDamage(float amount, float knockbackForce, Vector2 sourcePos);
    void fireProjectile();

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
