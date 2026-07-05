#pragma once
#include "EnemyState.h"
#include "EnemyStateMachine.h"
#include "core/entities/enemies/Enemy.h"

// ============================================================
//  EnemyStateDie  —  brief death animation then removal.
//  Main.cpp checks enemy.markedForRemoval each frame and
//  erases the enemy from the list.
// ============================================================
class EnemyStateDie : public EnemyState
{
public:
    float deathDuration = 0.5f;

private:
    float timer = 0.0f;

public:
    void enter() override
    {
        timer = deathDuration;
        enemy->velocity = {0.0f, 0.0f};
        enemy->meleeHitbox.isActive = false;
        enemy->hurtbox.setInvulnerable(true); // No more hits while dying
    }

    void physicsUpdate(float delta) override
    {
        timer -= delta;
        if (timer <= 0.0f)
            enemy->markedForRemoval = true;
    }
};
