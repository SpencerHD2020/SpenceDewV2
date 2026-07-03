#pragma once

// Forward declarations — concrete headers are included by the leaf state files.
class Player;
class StateMachine;

// ============================================================
//  State  —  base class for every player/enemy state.
//  Override only the hooks you need; all default to no-ops.
// ============================================================
class State {
public:
    StateMachine* stateMachine = nullptr;
    Player*       player       = nullptr;

    virtual ~State() = default;

    virtual void enter()                      {}
    virtual void exit()                       {}
    virtual void update(float /*delta*/)      {}
    virtual void physicsUpdate(float /*delta*/) {}
};
