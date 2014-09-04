#ifndef MAINMENU_H_INCLUDED
#define MAINMENU_H_INCLUDED

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include "StateMachine.h"
#include "Artist.h"
#include "Button.h"

#define MENU_OPTIONS_OFFSET 90

class MainMenu
{
    public:
      MainMenu(Artist* artistPointer, StateMachine* statePointer);

      void run();

      void setState(State);

    protected:
      StateMachine* stateMachine;
      Artist* artist;

      SDL_Event event;

      int numOptions;
      int place;

      Image title;
      Image animals;
      Image selector;
      Button save;
      Button newGame;
      Button continueGame;
      Button loadGame;
      Button editor;
      Button quit;

      State menuState;

      void cycle(std::string);
      void update();
      void display();
      void check_events();
      void setPositions();
};

#endif // MAINMENU_H_INCLUDED
