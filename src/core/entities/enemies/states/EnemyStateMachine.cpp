#include "core/entities/enemies/states/EnemyStateMachine.h"
#include <cstdio>

void EnemyStateMachine::init(Enemy *e, const std::string &initialState)
{
    enemy = e;
    for (auto &[name, s] : states)
        s->enemy = e;
    changeState(initialState);
}

void EnemyStateMachine::changeState(const std::string &name)
{
    auto it = states.find(name);
    if (it == states.end())
    {
        printf("ERROR: EnemyStateMachine — unknown state '%s'\n", name.c_str());
        return;
    }
    if (currentState && currentState == it->second.get())
        return;
    if (currentState)
        currentState->exit();
    currentState = it->second.get();
    currentState->enter();
}

void EnemyStateMachine::update(float delta)
{
    if (currentState)
        currentState->update(delta);
}

void EnemyStateMachine::physicsUpdate(float delta)
{
    if (currentState)
        currentState->physicsUpdate(delta);
}
