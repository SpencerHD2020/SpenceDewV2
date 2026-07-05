#include "core/entities/enemies/Enemy.h"
#include "core/entities/enemies/states/EnemyStateIdle.h"
#include "core/entities/enemies/states/EnemyStateChase.h"
#include "core/entities/enemies/states/EnemyStateAttack.h"
#include "core/entities/enemies/states/EnemyStateDie.h"
#include "raymath.h"
#include <cstdio>
#include <cmath>

void Enemy::init(Vector2 spawnPos)
{
    position = spawnPos;

    // --- Hurtbox ---
    hurtbox.ownerEntity = this;
    hurtbox.onDamageReceived = [this](float amount, float knockback, Vector2 sourcePos)
    {
        takeDamage(amount, knockback, sourcePos);
    };
    hurtbox.rect = getBounds();

    // --- Hitbox ---
    meleeHitbox.ownerEntity = this;
    meleeHitbox.damage = 8.0f;
    meleeHitbox.knockbackForce = 200.0f;
    meleeHitbox.isActive = false;

    // --- States ---
    fsm.addState<EnemyStateIdle>("idle");
    fsm.addState<EnemyStateChase>("chase");
    fsm.addState<EnemyStateAttack>("attack");
    fsm.addState<EnemyStateDie>("die");

    fsm.init(this, "idle");
}

void Enemy::update(float delta)
{
    if (!alive)
        return;
    fsm.update(delta);
    fsm.physicsUpdate(delta);
    hurtbox.rect = getBounds();
}

void Enemy::draw() const
{
    if (!alive)
        return;

    // Flash red/dark-red when in die state
    Color bodyColor = markedForRemoval ? BLANK : RED;
    if (markedForRemoval)
        return;

    // Tint darker during death state
    if (currentHealth <= 0.0f)
        bodyColor = MAROON;

    DrawCircleV(position, collisionRadius, bodyColor);

    // Facing indicator
    Vector2 facingEnd = {
        position.x + facing.x * (collisionRadius + 6.0f),
        position.y + facing.y * (collisionRadius + 6.0f)};
    DrawLineV(position, facingEnd, ORANGE);

    // Health bar (small, above enemy)
    float barW = collisionRadius * 2.0f + 4.0f;
    float barX = position.x - barW * 0.5f;
    float barY = position.y - collisionRadius - 10.0f;
    DrawRectangle(static_cast<int>(barX), static_cast<int>(barY),
                  static_cast<int>(barW), 4, DARKGRAY);
    float hpFrac = currentHealth / maxHealth;
    DrawRectangle(static_cast<int>(barX), static_cast<int>(barY),
                  static_cast<int>(barW * hpFrac), 4, GREEN);

    // Debug: active hitbox
    if (meleeHitbox.isActive)
        DrawRectangleLinesEx(meleeHitbox.rect, 2, ORANGE);
}

void Enemy::moveAndSlide(float delta)
{
    // Decay knockback
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

    // X axis
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

    // Y axis
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

void Enemy::takeDamage(float amount, float knockbackForce, Vector2 sourcePos)
{
    currentHealth -= amount;

    // Apply knockback
    Vector2 diff = Vector2Subtract(position, sourcePos);
    float dist = Vector2Length(diff);
    if (dist > 0.0f)
    {
        Vector2 kbDir = Vector2Scale(diff, 1.0f / dist);
        knockbackVelocity.x += kbDir.x * knockbackForce;
        knockbackVelocity.y += kbDir.y * knockbackForce;
    }

    if (onDamageVisual)
        onDamageVisual(position, amount);

    if (currentHealth <= 0.0f)
        onDeath();
}

void Enemy::onDeath()
{
    alive = false;
    meleeHitbox.isActive = false;
    fsm.changeState("die");
}
