#include "StateMachine.h"

StateMachine::StateMachine (State init)
{
    gameState = init;
}

State StateMachine::getState ()
{
    return gameState;
}

State& StateMachine::getStateRef()
{
    return gameState;
}

void StateMachine::setState (State set)
{
    gameState = set;
}
