#pragma once
#include "State.h"
#include "StateMachine.h"
#include "core/Input.h"
#include "core/entities/player/Player.h"

// ============================================================
//  StateCast  —  brief magic projectile launch.
//
//  On enter: fire a projectile in the player's facing direction
//  via player->fireProjectile() (callback registered in main.cpp).
//  Locks the player for castDuration seconds, then returns to
//  idle or run.
// ============================================================
class StateCast : public State
{
public:
    float castDuration = 0.25f; // Seconds of lockout (brief wind-up)
    float moveScale = 0.3f;     // Fraction of walk speed during cast (slight stutter)

private:
    float castTimer = 0.0f;
    bool fired = false;

public:
    void enter() override
    {
        castTimer = castDuration;
        fired = false;
        player->velocity = {0.0f, 0.0f};
        player->sprite.flipX = false;
        player->sprite.play("cast_down");
    }

    void physicsUpdate(float delta) override
    {
        castTimer -= delta;

        // Fire at the halfway point (feel of a brief wind-up)
        if (!fired && castTimer <= castDuration * 0.5f)
        {
            fired = true;
            player->fireProjectile();
        }

        // Slight forward drift during cast
        player->velocity = {
            player->facing.x * 30.0f * moveScale,
            player->facing.y * 30.0f * moveScale};
        player->moveAndSlide(delta);

        if (castTimer <= 0.0f)
        {
            returnToLocomotion();
        }
    }
};
