#pragma once
#include "EnemyState.h"
#include "EnemyStateMachine.h"
#include "core/entities/enemies/Enemy.h"
#include "raymath.h"

// ============================================================
//  EnemyStateIdle  —  enemy stands still and watches.
//  Transitions to Chase when the player enters aggroRange.
// ============================================================
class EnemyStateIdle : public EnemyState
{
public:
    void enter() override
    {
        enemy->velocity = {0.0f, 0.0f};
    }

    void physicsUpdate(float /*delta*/) override
    {
        if (!enemy->playerPos)
            return;

        float dist = Vector2Distance(enemy->position, *enemy->playerPos);
        if (dist <= enemy->aggroRange)
            stateMachine->changeState("chase");
    }
};
