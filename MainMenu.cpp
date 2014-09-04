#include "MainMenu.h"
#include <iostream>

MainMenu::MainMenu(Artist* artistPointer, StateMachine* statePointer)
{
  // cout statements will print to a stdout.txt file in /Debug/ or /Release/
  // If you are seeing it print to a console, make sure the project is set
  // to compile as a GUI application. ("properties" -> "build targets" in C::B)
  cout << "\n_______________MAIN MENU INITIALIZATION_______________\n";
  //points to the main state machine, holding what state the game is in
  stateMachine = statePointer;

  //points to the Artist, used clear the screen, draw stuff, etc.
  artist = artistPointer;

  // initialize our Button objects with vertical frame split and semitransparency
  newGame.initialize(artist, "menu/NewGame.png", false, true);
  continueGame.initialize(artist, "menu/Continue.png", false, true);
  editor.initialize(artist, "menu/Editor.png",false, true);
  quit.initialize(artist, "menu/Quit.png", false, true);

  // initialize the static images
  selector.initialize(artist, "menu/Selector.png");
  title.initialize(artist, "menu/title.png", true, WINDOW_WIDTH/2 - 220, 20);
  animals.initialize(artist, "menu/animals.png", true, 20, WINDOW_HEIGHT - 300);

  // this will never change
  selector.set_x(200);

  menuState=NEW_GAME;
}

///used by the main.cpp to prep the menu before running it
void MainMenu::setState(State newstate)
{
  menuState=newstate;
}

///main loop of the main menu
void MainMenu::run()
{
  // set initial positions, decide whether "continue" is available
  setPositions();

  while( stateMachine -> getState() == MAIN_MENU )
  {
    // get keyboard input
    check_events();

    // change positions of stuff
    update();

    // draw to the screen buffer
    display();

    // displays the new screen image
    artist -> flip();

    // check how fast the last frame took to create, and delay the
    // next one accordingly.
    // The argument specifies whether we want menu FPS or in-game FPS.
    artist -> framerate_limit(true);
  }
}

///called every iteration of the main loop to display the images
void MainMenu::display()
{
  SDL_Surface* screen = artist -> getScreen();

  // clearing will be unnecessary if we use a background graphic
  artist -> clear((Uint32) 0xfffc3e);

  title.display(screen);
  animals.display(screen);

  continueGame.force_up();
  newGame.force_up();
  editor.force_up();
  quit.force_up();

  switch(menuState)
  {
    case CONTINUE: continueGame.mouseover(); break;
    case NEW_GAME: newGame.mouseover(); break;
    case EDITOR: editor.mouseover(); break;
    case QUIT: quit.mouseover(); break;
  }

  // if "continue" is available
  if(numOptions>3)
    continueGame.display(screen);

  //selector.display(screen);
  newGame.display(screen);
  editor.display(screen);
  quit.display(screen);
}

///sets the initial positions of the options when we start the program or return to menu (pause)
void MainMenu::setPositions()
{
  // since sometimes "continue" will be an option and sometimes it won't,
  // numOptions is used for positioning options differently depending on this.
  numOptions=3;

  if (menuState == IN_GAME)
  {
    numOptions = 4;
    menuState = CONTINUE;
  }
  else
  {
    menuState = NEW_GAME;
  }

  continueGame.set_yOffset(MENU_OPTIONS_OFFSET + 300/numOptions);

  // If the "continue" option is unavailable, it will be on top
  // of "new game" but will not be displayed. "place" is used to offest
  // "continue" is available.

  place = 1;

  if(numOptions>3)
  {
    // offset everything else down
    // this is "numOptions - 2" in case we want to
    // add other options above it (like save/load)
    place = numOptions - 2;
  }

  newGame.set_yOffset(MENU_OPTIONS_OFFSET + 300*place/numOptions);
  editor.set_yOffset(MENU_OPTIONS_OFFSET + 300*(place+1)/numOptions);
  quit.set_yOffset(MENU_OPTIONS_OFFSET + 300*(place+2)/numOptions);

}

///reposition the menu options and selector
void MainMenu::update()
{
  // move the selector image to the currently selected option
  switch(menuState)
  {
    case CONTINUE: selector.set_y(MENU_OPTIONS_OFFSET + 300/numOptions); break;
    case NEW_GAME: selector.set_y(MENU_OPTIONS_OFFSET + 300*(place)/numOptions); break;
    case EDITOR: selector.set_y(MENU_OPTIONS_OFFSET + 300*(place+1)/numOptions); break;
    case QUIT: selector.set_y(MENU_OPTIONS_OFFSET + 300*(place+2)/numOptions); break;
    default: menuState = NEW_GAME;
      selector.set_y(MENU_OPTIONS_OFFSET + 300*(place+1)/numOptions);
  }
  // get a local representation of the selector image's bounds
  SDL_Rect box = selector.get_bounds();

  // ratio of vertical distance between selector & given option
  // to total vertical span of options
  double ratio;
  // the horizontal position a given option SHOULD be in
  double target;

  //distance from top to bottom positions
  int diff = (300*(place+2)/numOptions) - (300/numOptions);

  // holds pointers to all the option images
  /// PROTIP: Only use arrays for temporary stuff like this.
  ///         Normally use a class like std::vector or std::map
  Button* options[] = {&continueGame, &newGame, &editor, &quit};

  // set the x position for each option
  for(int i=0; i<4; i++)
  {
    ratio = (double)abs(box.y - options[i]->get_bounds().y)/(double)diff;
    target = box.x + box.w + 40 + 2*ratio*(280 - box.x - box.w);
    options[i]->set_xOffset(options[i]->get_bounds().x + ((int)target - options[i]->get_bounds().x)/12);
  }
}

///get keyboard input
void MainMenu::check_events(){
  // while an even is being registered with SDL
  int mx, my;
  bool mousedown = SDL_GetMouseState(&mx, &my)&SDL_BUTTON(1);
  while( SDL_PollEvent( &event ) )
  {
    // if the user clicks the x-out button or something
    if( event.type == SDL_QUIT )
    {
      // close the program politely
      stateMachine -> setState(QUIT);
      return;
    }
    else if( event.type == SDL_KEYDOWN )
    {
      switch(event.key.keysym.sym)
      {
        case SDLK_DOWN: cycle("DOWN"); break;
        case SDLK_UP: cycle("UP"); break;
        case SDLK_ESCAPE: cycle("ESC"); break;
        case SDLK_RETURN: cycle("ENTER"); break;
        default: break;
      }
    }
    else if(event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEMOTION)
    {
      if(my < MENU_OPTIONS_OFFSET*.9 + 300*(place)/numOptions && numOptions == 4)
      {
        menuState = CONTINUE;
      }
      else if(my < MENU_OPTIONS_OFFSET*.9 + 300*(place+1)/numOptions)
      {
        menuState = NEW_GAME;
      }
      else if(my < MENU_OPTIONS_OFFSET*.9 + 300*(place+2)/numOptions)
      {
        menuState = EDITOR;
      }
      else if(my < MENU_OPTIONS_OFFSET*.9 + 300*(place+3)/numOptions)
      {
        menuState = QUIT;
      }
      if(event.type == SDL_MOUSEBUTTONDOWN)
        cycle("ENTER");
    }
  }
}

///handle the keyboard input
void MainMenu::cycle(std::string dir)
{
  if(dir == "UP")
  {
    switch(menuState)
    {
      case CONTINUE: menuState = QUIT; break;
      case NEW_GAME: if(numOptions == 4)
                        menuState = CONTINUE;
                     else
                        menuState = QUIT;
                     break;
      case EDITOR: menuState = NEW_GAME; break; break;
      case QUIT: menuState = EDITOR; break; break;
      default: menuState = NEW_GAME;
    }
  }
  else if(dir == "DOWN")
  {
    switch(menuState)
    {
      case CONTINUE:  menuState = NEW_GAME; break;
      case NEW_GAME:  menuState = EDITOR; break;
      case EDITOR:    menuState = QUIT; break;
      case QUIT:      if(numOptions>3)
                        menuState = CONTINUE;
                      else
                        menuState = NEW_GAME;
                      break;
      default: menuState = NEW_GAME;
    }
  }
  else if(dir == "ESC")
  {
    if(menuState==QUIT)
      stateMachine -> setState(QUIT);
    else
      menuState=QUIT;
  }
  else if (dir == "ENTER")
  {
    stateMachine -> setState(menuState);
  }
}
