#include "State.h"
#include "StateMachine.h"
#include "core/Input.h"
#include "core/entities/player/Player.h"

bool State::checkActionTransitions()
{
    if (Input::attackJustPressed())
    {
        stateMachine->changeState("attack");
        return true;
    }
    if (Input::castJustPressed())
    {
        stateMachine->changeState("cast");
        return true;
    }
    if (Input::dodgeJustPressed())
    {
        stateMachine->changeState("dodge");
        return true;
    }
    return false;
}

void State::returnToLocomotion()
{
    Vector2 dir = Input::getMoveVector();
    stateMachine->changeState((dir.x != 0.0f || dir.y != 0.0f) ? "run" : "idle");
}
