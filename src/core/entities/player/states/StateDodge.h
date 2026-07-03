#pragma once
#include "State.h"
#include "StateMachine.h"
#include "core/Input.h"
#include "core/entities/player/Player.h"

// ============================================================
//  StateDodge  —  invulnerable roll in the input direction.
//  I-frames are granted for the full dodge duration.
// ============================================================
class StateDodge : public State {
public:
    float dodgeSpeed    = 400.0f;
    float dodgeDuration = 0.3f;

private:
    float   dodgeTimer = 0.0f;
    Vector2 dodgeDir   = {0.0f, 1.0f};

public:
    void enter() override {
        dodgeTimer = dodgeDuration;
        player->hurtbox.setInvulnerable(true);

        dodgeDir = Input::getMoveVector();
        if (dodgeDir.x == 0.0f && dodgeDir.y == 0.0f)
            dodgeDir = player->facing; // Dodge backwards if no directional input
    }

    void physicsUpdate(float delta) override {
        dodgeTimer -= delta;
        player->velocity = {dodgeDir.x * dodgeSpeed, dodgeDir.y * dodgeSpeed};
        player->moveAndSlide(delta);

        if (dodgeTimer <= 0.0f)
            stateMachine->changeState("idle");
    }

    void exit() override {
        player->hurtbox.setInvulnerable(false);
    }
};
