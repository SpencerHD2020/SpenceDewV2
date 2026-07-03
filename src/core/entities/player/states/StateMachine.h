#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <cstdio>
#include "State.h"

class Player;

// ============================================================
//  StateMachine  —  owns all states, drives the active one.
//
//  Usage:
//    fsm.addState<StateIdle>("idle");
//    fsm.addState<StateRun>("run");
//    fsm.init(playerPtr, "idle");   // must be called after addState
//
//  From inside a State:
//    stateMachine->changeState("run");
// ============================================================
class StateMachine {
public:
    Player* player       = nullptr;
    State*  currentState = nullptr;
    std::unordered_map<std::string, std::unique_ptr<State>> states;

    // Register a state. Returns a raw pointer for optional post-registration config.
    template<typename T>
    T* addState(const std::string& name) {
        auto s          = std::make_unique<T>();
        s->stateMachine = this;
        T* ptr          = s.get();
        states[name]    = std::move(s);
        return ptr;
    }

    // Set player reference on all states, then transition to initialState.
    void init(Player* p, const std::string& initialState);

    void changeState(const std::string& name);
    void update(float delta);
    void physicsUpdate(float delta);
};
