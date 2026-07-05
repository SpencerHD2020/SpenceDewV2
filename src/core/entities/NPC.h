#pragma once
#include "raylib.h"
#include "core/animation/AnimatedSprite.h"
#include <vector>

// ============================================================
//  NPC  —  shared base for every non-player character.
//
//  Holds only the "core" stuff every character needs: transform,
//  a sprite, a collision radius, a walls reference, and basic
//  wall-aware movement (moveAndSlide).
//
//  Deliberately NO combat and NO dialogue here:
//    - Enemy    : public NPC   → adds combat (hitbox/hurtbox, AI FSM)
//    - Friend   : public NPC   → adds interaction + dialogue
//    - PotentialPartner : public Friend → adds romance data
//
//  Subclasses override update()/draw() (and may override
//  moveAndSlide() for extra behaviour like knockback).
// ============================================================
class NPC
{
public:
    NPC() = default;
    virtual ~NPC() = default;
    NPC(NPC &&) = default;
    NPC &operator=(NPC &&) = default;
    NPC(const NPC &) = default;
    NPC &operator=(const NPC &) = default;

    // --- Transform ---
    Vector2 position = {0.0f, 0.0f};
    Vector2 velocity = {0.0f, 0.0f};
    Vector2 facing = {0.0f, 1.0f}; // Default: facing down

    // --- Collision ---
    float collisionRadius = 12.0f;

    // --- Environment (set by the owner after construction) ---
    const std::vector<Rectangle> *walls = nullptr;

    // --- Visuals ---
    AnimatedSprite sprite;

    Rectangle getBounds() const
    {
        return {
            position.x - collisionRadius,
            position.y - collisionRadius,
            collisionRadius * 2.0f,
            collisionRadius * 2.0f};
    }

    // Integrate velocity into position with axis-separated wall collision.
    // No knockback here — Enemy overrides this to layer combat knockback and
    // hurtbox syncing on top.
    virtual void moveAndSlide(float delta);

    virtual void update(float /*delta*/) {}
    virtual void draw() const {}
};
