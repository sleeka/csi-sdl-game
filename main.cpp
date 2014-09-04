#include "Engine.h"
#include "Editor.h"
#include "MainMenu.h"
#include "ScoreScreen.h"

int main(int argc, char* args[])
{
  cout << "main.cpp : stdout test successful.\n";

  bool quit = false;
  srand(time(NULL));

  // manages the screen and image manipulation stuff
  Artist artist(quit);

  // not really a true state machine,
  // basically just holds our main state
  StateMachine stateMachine;

  // this will allow us to create levels
  Editor editor(&artist, &stateMachine);

  // contains the main menu loop and stuff
  MainMenu mainMenu(&artist, &stateMachine);

  ScoreScreen scoreScreen(&artist, &stateMachine);

  // this will contain the actual "game code"
  Engine game(&artist, &stateMachine, &scoreScreen);

  // state loop
  while(!quit)
  {
    // states enumerated in helpers.h
    switch(stateMachine.getState())
    {
      // default state upon initializations
      case TITLE:     // do whatever splash/intro stuff we want.
                      // proceed to the main menu.
                      stateMachine.setState(MAIN_MENU); break;

      // execute the menu loop
      case MAIN_MENU: mainMenu.run(); break;

      // show the player's score
      case SCORESCREEN: scoreScreen.run(); break;

      case NEW_GAME:  // we will set up stuff for the game here
                      // like (re)setting player attributes
                      game.newGame(true);

      case CONTINUE:
                      // actual game loop, currently empty
                      game.run();
                      // this will make the "continue" option available
                      mainMenu.setState(IN_GAME);
                      break;

      case EDITOR:    editor.run();
                      mainMenu.setState(EDITOR);
                      break;

                      // break out of the state loop.
                      // fight tha powah!
      case QUIT:      quit = true; break;

      // IMPOSSIBRU! But good practice anyway.
      // We should never see the GAME_OVER state from here, as it will be
      // reset to main menu anyway once we break from the (Engine) game loop
      default: stateMachine.setState(MAIN_MENU); break;
    }
  }
}
