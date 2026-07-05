#pragma once
#include "raylib.h"
#include "core/systems/combat/Hitbox.h"
#include "core/systems/combat/Hurtbox.h"
#include "core/entities/enemies/states/EnemyStateMachine.h"
#include <vector>
#include <functional>

class Pathfinder;

// ============================================================
//  Enemy  —  basic hostile entity.
//
//  Drives an EnemyStateMachine with Idle → Chase → Attack → Die.
//  Main.cpp owns all Enemy instances in a std::vector and sets
//  the external pointers (playerPos, walls, pathfinder) after
//  construction.
// ============================================================
class Enemy
{
public:
    // Non-copyable (EnemyStateMachine holds unique_ptrs and self-referential pointers).
    Enemy() = default;
    Enemy(Enemy &&) = default;
    Enemy &operator=(Enemy &&) = default;
    Enemy(const Enemy &) = delete;
    Enemy &operator=(const Enemy &) = delete;

    // --- Transform ---
    Vector2 position = {};
    Vector2 velocity = {};
    Vector2 facing = {0.0f, 1.0f};

    // --- Stats ---
    float maxHealth = 50.0f;
    float currentHealth = 50.0f;
    float moveSpeed = 75.0f;
    float collisionRadius = 12.0f;

    // --- State flags ---
    bool alive = true;
    bool markedForRemoval = false;

    // --- Combat ---
    Hitbox meleeHitbox;
    Hurtbox hurtbox;

    // --- Knockback ---
    Vector2 knockbackVelocity = {};
    float knockbackDecay = 700.0f;

    // --- Detection radii ---
    float aggroRange = 220.0f;
    float attackRange = 38.0f;
    float deaggroRange = 320.0f;

    // --- External scene references (set by main.cpp before first update) ---
    const Vector2 *playerPos = nullptr;
    const std::vector<Rectangle> *walls = nullptr;
    const Pathfinder *pathfinder = nullptr;

    // --- FSM ---
    EnemyStateMachine fsm;

    // --- Callbacks ---
    std::function<void(Vector2, float)> onDamageVisual; // spawn damage number

    void init(Vector2 spawnPos);
    void update(float delta);
    void draw() const;

    void moveAndSlide(float delta);
    void takeDamage(float amount, float knockbackForce, Vector2 sourcePos);

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
