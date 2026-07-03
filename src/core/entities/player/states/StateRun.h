#pragma once
#include "State.h"
#include "StateMachine.h"
#include "core/Input.h"
#include "core/entities/player/Player.h"

// ============================================================
//  StateRun  —  player is moving.
//  Walk / sprint speeds, transitions to idle / attack / dodge.
// ============================================================
class StateRun : public State {
public:
    float walkSpeed   = 150.0f;
    float sprintSpeed = 250.0f;

    void physicsUpdate(float delta) override {
        if (Input::attackJustPressed()) { stateMachine->changeState("attack"); return; }
        if (Input::dodgeJustPressed())  { stateMachine->changeState("dodge");  return; }

        Vector2 dir = Input::getMoveVector();
        if (dir.x == 0.0f && dir.y == 0.0f) {
            stateMachine->changeState("idle");
            return;
        }

        player->facing   = dir;
        float speed      = Input::sprintHeld() ? sprintSpeed : walkSpeed;
        player->velocity = {dir.x * speed, dir.y * speed};
        player->moveAndSlide(delta);
    }
};
