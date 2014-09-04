#ifndef SCORESCREEN_H_INCLUDED
#define SCORESCREEN_H_INCLUDED

#include "helpers.h"
#include "Artist.h"
#include "StateMachine.h"
#include "Button.h"

class ScoreScreen
{
  public:
    ScoreScreen(Artist* artistPointer, StateMachine* statePointer);
    void run();
    void updateTimeScore(int sc);
    void updateCompletion(int sc);
    void gradeTime( int par );

  private:
    StateMachine* stateMachine;
    Artist* artist;

    SDL_Event event;

    Button continueButton;
    Image scoreBox;
    Image timeGrade;
    Image scoreGrade;
    Image overallGrade;

    int timeScore, completionScore, overallScore;

    void check_events();
    void display();

    void calcOverallScore(int grade);
};

#endif // SCORESCREEN_H_INCLUDED
