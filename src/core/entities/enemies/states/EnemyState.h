#pragma once

class Enemy;
class EnemyStateMachine;

// ============================================================
//  EnemyState  —  base class for every enemy state.
//  Mirrors the player State pattern but typed for Enemy.
// ============================================================
class EnemyState
{
public:
    EnemyStateMachine *stateMachine = nullptr;
    Enemy *enemy = nullptr;

    virtual ~EnemyState() = default;

    virtual void enter() {}
    virtual void exit() {}
    virtual void update(float /*delta*/) {}
    virtual void physicsUpdate(float /*delta*/) {}
};
