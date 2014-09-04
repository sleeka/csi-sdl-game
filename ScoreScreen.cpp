#include "ScoreScreen.h"
#include <iostream>

using namespace std;

ScoreScreen::ScoreScreen(Artist* artistPointer, StateMachine* statePointer)
{
  artist = artistPointer;
  stateMachine = statePointer;
  continueButton.initialize(artist, "menu/Continue.png", false, true, WINDOW_WIDTH/2 - 130, WINDOW_HEIGHT - 160);
  scoreBox.initialize(artist, "menu/scorebox.png", true, WINDOW_WIDTH/2 - 195, 110);

  timeGrade.initialize(artist, "grades.png", true, WINDOW_WIDTH/2 + 120, WINDOW_HEIGHT/2 - 102);
  scoreGrade.initialize(artist, "grades.png", true, WINDOW_WIDTH/2 + 120, WINDOW_HEIGHT/2 - 57);
  overallGrade.initialize(artist, "grades.png", true, WINDOW_WIDTH/2 + 120, WINDOW_HEIGHT/2 - 7);

  int h =  artist->sprite_height("sprites/grades.png") ;
  timeGrade.frameSplice(h, h);
  scoreGrade.frameSplice(h, h);
  overallGrade.frameSplice(h, h);

  overallScore = -1;
}

///main loop of the scorescreen
void ScoreScreen::run()
{
  while( stateMachine -> getState() == SCORESCREEN )
  {
    // get keyboard input
    check_events();

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

void ScoreScreen::updateTimeScore(int sc)
{
  timeScore = sc;
}

void ScoreScreen::updateCompletion(int sc)
{
  completionScore = sc;

  // 100% is A+, 99 - 90 is B, etc.
  int letter = (109 - completionScore) / 10;
  if(letter > 5)
   letter = 5;
  scoreGrade.setFrame( letter );
  calcOverallScore( letter );
}

void ScoreScreen::gradeTime( int par )
{
  int step;
  if( par > 9)
    step = par/5;
  else
    step = 1;

  int i = 0;
  for(; i<5 && timeScore > par + step*i; i++);

  timeGrade.setFrame(i);
  calcOverallScore(i);
}

void ScoreScreen::calcOverallScore( int grade )
{
  if( overallScore < 0 )
    overallScore = grade;
  else
  {
    cout << "score 1 = " << overallScore << " , score 2 = " << grade;
    if((overallScore + grade) % 2 == 1)
    {
      grade++;
      cout << " adding one to score 2 to make it even, ";
    }
    // average the two grades
    overallScore = (overallScore + grade)/2;
    overallGrade.setFrame( overallScore );
    cout << "\noverall score set to " << overallScore << "\n";
    overallScore = -1;
  }
}

// magic numbers are just positioning on the screen
void ScoreScreen::display()
{
  // clear to the bright yellow background color
  artist->clear((Uint32) 0xfffc3e);
  // show the textbox image
  scoreBox.display(artist->getScreen());
  // display the time in seconds
  artist->display_numbers(timeScore, WINDOW_WIDTH/2 - 5, WINDOW_HEIGHT/2 - 75);
  // display the percentage of the score achieved
  artist->display_numbers(completionScore, WINDOW_WIDTH/2 - 5, WINDOW_HEIGHT/2 - 32, true);

  timeGrade.display(artist->getScreen());
  scoreGrade.display(artist->getScreen());
  overallGrade.display(artist->getScreen());
  continueButton.display(artist->getScreen());
}

void ScoreScreen::check_events(){
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
        case SDLK_q: // both 'q' and 'ESC' pop us back to the main menu
        case SDLK_ESCAPE: stateMachine -> setState(MAIN_MENU); break;
        default: break;
      }
    }
    else if(event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEMOTION)
    {
      SDL_Rect continue_bounds = continueButton.get_bounds();
      if(mx >= continue_bounds.x && mx <= continue_bounds.x + continue_bounds.w && my > continue_bounds.y && my < continue_bounds.y + continue_bounds.h/2)
      {
        continueButton.mouseover();
        if(event.type == SDL_MOUSEBUTTONDOWN)
        {
          stateMachine->setState(CONTINUE);
        }
      }
      else
        continueButton.mouseoff();
    }
  }
}
