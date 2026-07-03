#pragma once
#include "State.h"
#include "StateMachine.h"
#include "core/Input.h"
#include "core/entities/player/Player.h"

// ============================================================
//  StateIdle  —  player is standing still.
//  Watches for attack, dodge, or movement input.
// ============================================================
class StateIdle : public State {
public:
    void enter() override {
        player->velocity = {0.0f, 0.0f};
    }

    void physicsUpdate(float /*delta*/) override {
        if (Input::attackJustPressed()) { stateMachine->changeState("attack"); return; }
        if (Input::dodgeJustPressed())  { stateMachine->changeState("dodge");  return; }

        Vector2 dir = Input::getMoveVector();
        if (dir.x != 0.0f || dir.y != 0.0f) {
            stateMachine->changeState("run");
        }
    }
};
