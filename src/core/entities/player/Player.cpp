#include "core/entities/player/Player.h"

// Include all state headers here so addState<T>() can instantiate them.
#include "core/entities/player/states/StateIdle.h"
#include "core/entities/player/states/StateRun.h"
#include "core/entities/player/states/StateAttack.h"
#include "core/entities/player/states/StateDodge.h"

#include <cstdio>

void Player::init()
{
    // --- Hurtbox setup ---
    hurtbox.ownerEntity = this;
    hurtbox.onDamageReceived = [this](float amount, Vector2 sourcePos)
    {
        takeDamage(amount, sourcePos);
    };
    hurtbox.rect = getBounds();

    // --- Melee hitbox setup ---
    meleeHitbox.ownerEntity = this;
    meleeHitbox.damage = 10.0f;
    meleeHitbox.isActive = false;

    // --- Register states ---
    fsm.addState<StateIdle>("idle");
    fsm.addState<StateRun>("run");
    fsm.addState<StateAttack>("attack");
    fsm.addState<StateDodge>("dodge");

    // Start in idle (also distributes the player ptr to all states)
    fsm.init(this, "idle");
}

void Player::update(float delta)
{
    fsm.update(delta);
    fsm.physicsUpdate(delta);
}

void Player::draw()
{
    // Placeholder visuals — replace with sprite once assets are ready
    DrawCircleV(position, collisionRadius, BLUE);

    // Facing indicator
    Vector2 facingEnd = {
        position.x + facing.x * (collisionRadius + 8.0f),
        position.y + facing.y * (collisionRadius + 8.0f)};
    DrawLineV(position, facingEnd, YELLOW);

    // Debug: active melee hitbox
    if (meleeHitbox.isActive)
    {
        DrawRectangleLinesEx(meleeHitbox.rect, 2, RED);
        DrawText("HIT", static_cast<int>(meleeHitbox.rect.x),
                 static_cast<int>(meleeHitbox.rect.y) - 14, 12, RED);
    }

    // Debug: hurtbox boundary
    if (hurtbox.isInvulnerable)
    {
        DrawRectangleLinesEx(hurtbox.rect, 2, GOLD); // gold = I-frames active
    }
}

void Player::moveAndSlide(float delta)
{
    // --- X axis ---
    position.x += velocity.x * delta;
    if (walls)
    {
        Rectangle bounds = getBounds();
        for (const Rectangle &wall : *walls)
        {
            if (CheckCollisionRecs(bounds, wall))
            {
                if (velocity.x > 0.0f)
                    position.x = wall.x - collisionRadius;
                else
                    position.x = wall.x + wall.width + collisionRadius;
                velocity.x = 0.0f;
                bounds = getBounds();
            }
        }
    }

    // --- Y axis ---
    position.y += velocity.y * delta;
    if (walls)
    {
        Rectangle bounds = getBounds();
        for (const Rectangle &wall : *walls)
        {
            if (CheckCollisionRecs(bounds, wall))
            {
                if (velocity.y > 0.0f)
                    position.y = wall.y - collisionRadius;
                else
                    position.y = wall.y + wall.height + collisionRadius;
                velocity.y = 0.0f;
                bounds = getBounds();
            }
        }
    }

    hurtbox.rect = getBounds();
}

void Player::takeDamage(float amount, Vector2 /*sourcePos*/)
{
    currentHealth -= amount;
    printf("Player took %.1f damage! Health: %.1f / %.1f\n",
           amount, currentHealth, maxHealth);
    if (currentHealth <= 0.0f)
        onDeath();
}

void Player::onDeath()
{
    printf("Player died! Respawning at origin.\n");
    currentHealth = maxHealth;
    position = {0.0f, 0.0f};
    velocity = {0.0f, 0.0f};
    fsm.changeState("idle");
}
