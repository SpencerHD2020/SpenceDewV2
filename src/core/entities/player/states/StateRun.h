#pragma once
#include "State.h"
#include "StateMachine.h"
#include "core/Input.h"
#include "core/entities/player/Player.h"
#include <cmath>

// ============================================================
//  StateRun  —  player is moving.
//  Walk / sprint speeds, transitions to idle / attack / dodge.
// ============================================================
class StateRun : public State
{
public:
    float walkSpeed = 150.0f;
    float sprintSpeed = 250.0f;

    void physicsUpdate(float delta) override
    {
        if (checkActionTransitions())
            return;

        Vector2 dir = Input::getMoveVector();
        if (dir.x == 0.0f && dir.y == 0.0f)
        {
            stateMachine->changeState("idle");
            return;
        }

        player->facing = dir;
        // Pick run animation: horizontal movement → run_left row (flip for right),
        // vertical movement → run_down row.
        if (std::abs(dir.x) >= std::abs(dir.y))
        {
            player->sprite.flipX = (dir.x > 0.0f);
            player->sprite.play("run_left");
        }
        else
        {
            player->sprite.flipX = false;
            player->sprite.play("run_down");
        }
        float speed = Input::sprintHeld() ? sprintSpeed : walkSpeed;
        player->velocity = {dir.x * speed, dir.y * speed};
        player->moveAndSlide(delta);
    }
};
