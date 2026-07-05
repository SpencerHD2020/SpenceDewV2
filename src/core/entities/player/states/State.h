#pragma once

// Forward declarations — concrete headers are included by the leaf state files.
class Player;
class StateMachine;

// ============================================================
//  State  —  base class for every player/enemy state.
//  Override only the hooks you need; all default to no-ops.
// ============================================================
class State
{
public:
    StateMachine *stateMachine = nullptr;
    Player *player = nullptr;

    virtual ~State() = default;

    virtual void enter() {}
    virtual void exit() {}
    virtual void update(float /*delta*/) {}
    virtual void physicsUpdate(float /*delta*/) {}

protected:
    // ---- Shared transition helpers (defined in State.cpp) ----
    // These live on the base class so leaf states don't each re-implement the
    // same input→state plumbing.

    // Checks the action inputs (attack / cast / dodge) and transitions to the
    // matching state. Returns true if a transition happened, so callers can
    // early-out of their physicsUpdate.
    bool checkActionTransitions();

    // Transitions to "run" or "idle" depending on whether movement input is
    // currently held. Used by states that hand control back to locomotion
    // (attack, cast).
    void returnToLocomotion();
};
