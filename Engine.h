#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

#include "StateMachine.h"
#include "Artist.h"
#include "Camera.h"
#include "Map.h"
#include "Player.h"
#include "ScoreScreen.h"
#include "SoundBoard.h"
#include "SDL/SDL_gfxPrimitives.h"

#define TERMINAL_VELOCITY 45
#define SPEED_MODIFIER 6
#define SCORE_XPOS 20
#define SCORE_YPOS WINDOW_HEIGHT-20

class Engine
{
  public:
    Engine(Artist* artistPointer, StateMachine* statePointer, ScoreScreen* scoreScreen);

    void run();
    void newGame(bool startOver = false);

  protected:
    StateMachine* stateMachine;
    ScoreScreen* scoreScreen;
    Artist* artist;
    Camera camera;
    Map map;
    SoundBoard soundBoard;

    Uint32 level_timer;

    // for displaying the player score
    Image border_left;
    Image border_right;

    SDL_Surface* screen;
    SDL_Event event;

    int gravity_timer;
    // width of a digit of the numbers sprite (in pixels)
    SDL_Rect digit_size;
    string nextMap;

    Player player;

    void update();
    void player_move_normally();
    void player_move_held();
    void player_launch();

    void validateCamera();

    void display();
    void drawScore();

    void input();
};

#endif // ENGINE_H_INCLUDED
