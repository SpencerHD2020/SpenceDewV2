#pragma once
#include "EnemyState.h"
#include "EnemyStateMachine.h"
#include "core/entities/enemies/Enemy.h"
#include "core/systems/navigation/Pathfinder.h"
#include "raymath.h"
#include <vector>

// ============================================================
//  EnemyStateChase  —  enemy navigates toward the player.
//
//  Uses A* via enemy->pathfinder when available.
//  Path is recalculated every pathInterval seconds.
//  Falls back to direct movement when the path is empty.
// ============================================================
class EnemyStateChase : public EnemyState
{
public:
    float pathInterval = 0.35f;  // Seconds between A* calls
    float waypointReach = 20.0f; // Distance to consider a waypoint reached

private:
    float pathTimer = 0.0f;
    std::vector<Vector2> path;
    int pathIdx = 0;

public:
    void enter() override
    {
        path.clear();
        pathIdx = 0;
        pathTimer = pathInterval; // Recalc immediately on first physicsUpdate
    }

    void physicsUpdate(float delta) override
    {
        if (!enemy->playerPos)
            return;

        float distToPlayer = Vector2Distance(enemy->position, *enemy->playerPos);

        // --- Transition checks ---
        if (distToPlayer <= enemy->attackRange)
        {
            stateMachine->changeState("attack");
            return;
        }
        if (distToPlayer > enemy->deaggroRange)
        {
            stateMachine->changeState("idle");
            return;
        }

        // --- Path refresh ---
        pathTimer -= delta;
        if (pathTimer <= 0.0f)
        {
            pathTimer = pathInterval;
            refreshPath();
        }

        // --- Follow path ---
        Vector2 moveDir = {0.0f, 0.0f};

        if (!path.empty() && pathIdx < static_cast<int>(path.size()))
        {
            Vector2 target = path[pathIdx];
            Vector2 toWp = Vector2Subtract(target, enemy->position);
            float wpDist = Vector2Length(toWp);

            if (wpDist <= waypointReach)
            {
                ++pathIdx;
            }
            else
            {
                moveDir = Vector2Scale(toWp, 1.0f / wpDist);
            }
        }
        else
        {
            // No path (open area or pathfinder unavailable): move directly
            Vector2 toPlayer = Vector2Subtract(*enemy->playerPos, enemy->position);
            float dist = Vector2Length(toPlayer);
            if (dist > 0.0f)
                moveDir = Vector2Scale(toPlayer, 1.0f / dist);
        }

        if (moveDir.x != 0.0f || moveDir.y != 0.0f)
            enemy->facing = moveDir;

        enemy->velocity = {moveDir.x * enemy->moveSpeed,
                           moveDir.y * enemy->moveSpeed};
        enemy->moveAndSlide(delta);
    }

private:
    void refreshPath()
    {
        path.clear();
        pathIdx = 0;

        if (!enemy->pathfinder || !enemy->playerPos)
            return;

        path = enemy->pathfinder->findPath(enemy->position, *enemy->playerPos);
        pathIdx = 0;

        // Skip the first waypoint if it's almost where we already are
        if (!path.empty() &&
            Vector2Distance(enemy->position, path[0]) < waypointReach)
        {
            pathIdx = 1;
        }
    }
};
