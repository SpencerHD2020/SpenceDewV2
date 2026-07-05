#pragma once
#include "State.h"
#include "StateMachine.h"
#include "core/Input.h"
#include "core/entities/player/Player.h"

// ============================================================
//  StateAttack  —  melee swing.
//  Activates the player's meleeHitbox at the right frame window,
//  then returns to idle or run.
// ============================================================
class StateAttack : public State
{
public:
    float attackDuration = 0.4f;  // Total state length (seconds)
    float attackSpeed = 50.0f;    // Slight lunge forward
    float hitboxStart = 0.15f;    // Seconds into attack when hitbox activates
    float hitboxDuration = 0.15f; // How long hitbox stays active
    float hitboxOffset = 60.0f;   // Distance from player centre
    float hitboxSize = 40.0f;     // Width & height of hitbox rect

private:
    float attackTimer = 0.0f;
    bool hitboxActive = false;
    Vector2 attackDir = {0.0f, 1.0f};

public:
    void enter() override
    {
        attackTimer = attackDuration;
        hitboxActive = false;

        Vector2 inputDir = Input::getMoveVector();
        attackDir = (inputDir.x != 0.0f || inputDir.y != 0.0f) ? inputDir : player->facing;
        player->facing = attackDir;
        player->sprite.flipX = false;
        player->sprite.play("attack_down");
    }

    void physicsUpdate(float delta) override
    {
        attackTimer -= delta;
        float elapsed = attackDuration - attackTimer;

        if (!hitboxActive && elapsed >= hitboxStart)
            activateHitbox();

        if (hitboxActive && elapsed >= hitboxStart + hitboxDuration)
            deactivateHitbox();

        player->velocity = {attackDir.x * attackSpeed, attackDir.y * attackSpeed};
        player->moveAndSlide(delta);

        if (attackTimer <= 0.0f)
        {
            deactivateHitbox();
            returnToLocomotion();
        }
    }

    void exit() override { deactivateHitbox(); }

private:
    void activateHitbox()
    {
        hitboxActive = true;
        player->meleeHitbox.isActive = true;
        float cx = player->position.x + attackDir.x * hitboxOffset - hitboxSize * 0.5f;
        float cy = player->position.y + attackDir.y * hitboxOffset - hitboxSize * 0.5f;
        player->meleeHitbox.rect = {cx, cy, hitboxSize, hitboxSize};
    }

    void deactivateHitbox()
    {
        if (hitboxActive)
        {
            hitboxActive = false;
            player->meleeHitbox.isActive = false;
        }
    }
};
