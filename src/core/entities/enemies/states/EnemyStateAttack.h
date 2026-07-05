#pragma once
#include "EnemyState.h"
#include "EnemyStateMachine.h"
#include "core/entities/enemies/Enemy.h"
#include "raymath.h"

// ============================================================
//  EnemyStateAttack  —  melee swing toward the player.
//
//  Activates the enemy's meleeHitbox during the hit window,
//  then returns to Chase.
// ============================================================
class EnemyStateAttack : public EnemyState
{
public:
    float attackDuration = 0.55f;
    float hitboxStart = 0.20f;
    float hitboxDuration = 0.18f;
    float hitboxOffset = 50.0f;
    float hitboxSize = 36.0f;
    float lungeFwd = 40.0f; // Forward movement during swing

private:
    float timer = 0.0f;
    bool hitActive = false;
    Vector2 attackDir = {0.0f, 1.0f};

public:
    void enter() override
    {
        timer = attackDuration;
        hitActive = false;
        enemy->velocity = {0.0f, 0.0f};

        // Lock attack direction onto current facing/player position
        if (enemy->playerPos)
        {
            Vector2 toPlayer = Vector2Subtract(*enemy->playerPos, enemy->position);
            float dist = Vector2Length(toPlayer);
            attackDir = (dist > 0.0f) ? Vector2Scale(toPlayer, 1.0f / dist) : enemy->facing;
        }
        else
        {
            attackDir = enemy->facing;
        }
    }

    void physicsUpdate(float delta) override
    {
        timer -= delta;
        float elapsed = attackDuration - timer;

        if (!hitActive && elapsed >= hitboxStart)
            activateHitbox();

        if (hitActive && elapsed >= hitboxStart + hitboxDuration)
            deactivateHitbox();

        // Brief lunge forward during swing
        enemy->velocity = {attackDir.x * lungeFwd, attackDir.y * lungeFwd};
        enemy->moveAndSlide(delta);

        if (timer <= 0.0f)
        {
            deactivateHitbox();
            stateMachine->changeState("chase");
        }
    }

    void exit() override { deactivateHitbox(); }

private:
    void activateHitbox()
    {
        hitActive = true;
        enemy->meleeHitbox.isActive = true;
        float cx = enemy->position.x + attackDir.x * hitboxOffset - hitboxSize * 0.5f;
        float cy = enemy->position.y + attackDir.y * hitboxOffset - hitboxSize * 0.5f;
        enemy->meleeHitbox.rect = {cx, cy, hitboxSize, hitboxSize};
    }

    void deactivateHitbox()
    {
        if (hitActive)
        {
            hitActive = false;
            enemy->meleeHitbox.isActive = false;
        }
    }
};
