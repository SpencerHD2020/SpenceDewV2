#include "core/entities/player/states/StateMachine.h"

void StateMachine::init(Player* p, const std::string& initialState) {
    player = p;
    for (auto& [name, s] : states) {
        s->player = p;
    }
    changeState(initialState);
}

void StateMachine::changeState(const std::string& name) {
    auto it = states.find(name);
    if (it == states.end()) {
        printf("ERROR: StateMachine — unknown state '%s'\n", name.c_str());
        return;
    }
    if (currentState && currentState == it->second.get()) return;
    if (currentState) currentState->exit();
    currentState = it->second.get();
    printf("---> State: %s\n", name.c_str());
    currentState->enter();
}

void StateMachine::update(float delta) {
    if (currentState) currentState->update(delta);
}

void StateMachine::physicsUpdate(float delta) {
    if (currentState) currentState->physicsUpdate(delta);
}
