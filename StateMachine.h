#ifndef STATEMACHINE_H_INCLUDED
#define STATEMACHINE_H_INCLUDED

#include "helpers.h"

class StateMachine{

  private:

    State gameState;

  public:

    StateMachine (State = TITLE);
    State getState ();
    State& getStateRef();
    void setState (State);

};

#endif // STATEMACHINE_H_INCLUDED
