#include "core/entities/player/Player.h"
#include "raymath.h"

// Include all state headers here so addState<T>() can instantiate them.
#include "core/entities/player/states/StateIdle.h"
#include "core/entities/player/states/StateRun.h"
#include "core/entities/player/states/StateAttack.h"
#include "core/entities/player/states/StateDodge.h"
#include "core/entities/player/states/StateCast.h"

#include <cstdio>
#include <cmath>

void Player::init()
{
    // --- Hurtbox setup ---
    hurtbox.ownerEntity = this;
    hurtbox.onDamageReceived = [this](float amount, float knockback, Vector2 sourcePos)
    {
        takeDamage(amount, knockback, sourcePos);
    };
    hurtbox.rect = getBounds();

    // --- Melee hitbox setup ---
    meleeHitbox.ownerEntity = this;
    meleeHitbox.damage = 10.0f;
    meleeHitbox.knockbackForce = 250.0f;
    meleeHitbox.isActive = false;

    // --- Sprite ---
    // Load before fsm.init() so animation defs exist when StateIdle::enter()
    // calls sprite.play(). JSON name differs from PNG, so pass explicitly.
    sprite.load("assets/sprites/character-spritesheet.png", "assets/sprites/character-spritesheet.json");

    // --- Register states ---
    fsm.addState<StateIdle>("idle");
    fsm.addState<StateRun>("run");
    fsm.addState<StateAttack>("attack");
    fsm.addState<StateDodge>("dodge");
    fsm.addState<StateCast>("cast");

    fsm.init(this, "idle");
}

void Player::update(float delta)
{
    // --- I-frame countdown ---
    if (iFrameTimer > 0.0f)
    {
        iFrameTimer -= delta;
        if (iFrameTimer <= 0.0f)
        {
            iFrameTimer = 0.0f;
            hurtbox.setInvulnerable(false);
        }
    }

    sprite.update(delta);
    fsm.update(delta);
    fsm.physicsUpdate(delta);
}

void Player::draw()
{
    // Flicker during I-frames (alpha pulses between ~50% and 100%)
    Color tint = WHITE;
    if (iFrameTimer > 0.0f)
    {
        unsigned char alpha = static_cast<unsigned char>(
            128.0f + 127.0f * std::sin(iFrameTimer * 30.0f));
        tint = {255, 255, 255, alpha};
    }

    // LPC sheet frames are 64px; draw at half size so the character reads at a
    // sensible scale next to the 16px dungeon tiles (temporary art).
    sprite.draw(position, 0.5f, tint);

    // Debug: active melee hitbox
    if (meleeHitbox.isActive)
    {
        DrawRectangleLinesEx(meleeHitbox.rect, 2, RED);
        DrawText("HIT", static_cast<int>(meleeHitbox.rect.x),
                 static_cast<int>(meleeHitbox.rect.y) - 14, 12, RED);
    }
}

void Player::moveAndSlide(float delta)
{
    // --- Decay knockback velocity ---
    float kbSpeed = Vector2Length(knockbackVelocity);
    if (kbSpeed > 0.0f)
    {
        float decel = knockbackDecay * delta;
        if (decel >= kbSpeed)
        {
            knockbackVelocity = {0.0f, 0.0f};
        }
        else
        {
            Vector2 kbDir = Vector2Normalize(knockbackVelocity);
            knockbackVelocity.x -= kbDir.x * decel;
            knockbackVelocity.y -= kbDir.y * decel;
        }
    }

    float vx = velocity.x + knockbackVelocity.x;
    float vy = velocity.y + knockbackVelocity.y;

    // --- X axis ---
    position.x += vx * delta;
    if (walls)
    {
        Rectangle bounds = getBounds();
        for (const Rectangle &wall : *walls)
        {
            if (CheckCollisionRecs(bounds, wall))
            {
                if (vx > 0.0f)
                    position.x = wall.x - collisionRadius;
                else
                    position.x = wall.x + wall.width + collisionRadius;
                knockbackVelocity.x = 0.0f;
                velocity.x = 0.0f;
                bounds = getBounds();
            }
        }
    }

    // --- Y axis ---
    position.y += vy * delta;
    if (walls)
    {
        Rectangle bounds = getBounds();
        for (const Rectangle &wall : *walls)
        {
            if (CheckCollisionRecs(bounds, wall))
            {
                if (vy > 0.0f)
                    position.y = wall.y - collisionRadius;
                else
                    position.y = wall.y + wall.height + collisionRadius;
                knockbackVelocity.y = 0.0f;
                velocity.y = 0.0f;
                bounds = getBounds();
            }
        }
    }

    hurtbox.rect = getBounds();
}

void Player::takeDamage(float amount, float knockbackForce, Vector2 sourcePos)
{
    currentHealth -= amount;
    printf("Player took %.1f damage! Health: %.1f / %.1f\n",
           amount, currentHealth, maxHealth);

    // Apply knockback impulse away from source
    Vector2 diff = Vector2Subtract(position, sourcePos);
    float dist = Vector2Length(diff);
    if (dist > 0.0f)
    {
        Vector2 kbDir = Vector2Scale(diff, 1.0f / dist);
        knockbackVelocity.x += kbDir.x * knockbackForce;
        knockbackVelocity.y += kbDir.y * knockbackForce;
    }

    // Grant I-frames
    hurtbox.setInvulnerable(true);
    iFrameTimer = iFrameDuration;

    if (onDamageVisual)
        onDamageVisual(position, amount);

    if (currentHealth <= 0.0f)
        onDeath();
}

void Player::fireProjectile()
{
    if (onFireProjectile)
        onFireProjectile(position, facing);
}

void Player::onDeath()
{
    printf("Player died! Respawning at origin.\n");
    currentHealth = maxHealth;
    position = {0.0f, 0.0f};
    velocity = {0.0f, 0.0f};
    knockbackVelocity = {0.0f, 0.0f};
    iFrameTimer = 0.0f;
    hurtbox.setInvulnerable(false);
    fsm.changeState("idle");
}
