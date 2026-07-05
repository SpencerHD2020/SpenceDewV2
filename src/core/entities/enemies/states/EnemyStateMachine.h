#pragma once
#include "EnemyState.h"
#include <memory>
#include <string>
#include <unordered_map>

class Enemy;

// ============================================================
//  EnemyStateMachine  —  mirrors StateMachine but typed for Enemy.
// ============================================================
class EnemyStateMachine
{
public:
    // Non-copyable; the states map holds unique_ptrs.
    EnemyStateMachine() = default;
    EnemyStateMachine(EnemyStateMachine &&) = default;
    EnemyStateMachine &operator=(EnemyStateMachine &&) = default;
    EnemyStateMachine(const EnemyStateMachine &) = delete;
    EnemyStateMachine &operator=(const EnemyStateMachine &) = delete;

    Enemy *enemy = nullptr;
    EnemyState *currentState = nullptr;
    std::unordered_map<std::string, std::unique_ptr<EnemyState>> states;

    template <typename T>
    T *addState(const std::string &name)
    {
        auto s = std::make_unique<T>();
        s->stateMachine = this;
        T *ptr = s.get();
        states[name] = std::move(s);
        return ptr;
    }

    void init(Enemy *e, const std::string &initialState);
    void changeState(const std::string &name);
    void update(float delta);
    void physicsUpdate(float delta);
};
