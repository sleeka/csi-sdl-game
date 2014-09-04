#include "Engine.h"

Engine::Engine(Artist* artistPointer, StateMachine* statePointer, ScoreScreen* scorePointer)
{
  cout << "\n_______________ENGINE INITIALIZATION_______________\n";
  stateMachine = statePointer;
  scoreScreen = scorePointer;
  artist = artistPointer;
  screen = artist -> getScreen();

  border_left.initialize(artist, "border-left.png", true, SCORE_XPOS, SCORE_YPOS);
  border_right.initialize(artist, "border-right.png", true, SCORE_XPOS, SCORE_YPOS);

  digit_size.w = artist->sprite_width("sprites/numbers.png") / 10;
  digit_size.h = artist->sprite_height("sprites/numbers.png");

  player.set_image(*artist);
  player.setSoundBoard(&soundBoard);

  nextMap = "start.map";

  soundBoard.addEffect("sound/bounce.wav");
  soundBoard.addEffect("sound/invuln.wav");
  soundBoard.addEffect("sound/points.wav");
  soundBoard.addEffect("sound/slowtime.wav");
  soundBoard.addEffect("sound/invuln_off.wav");
  soundBoard.addEffect("sound/chime.wav");
  soundBoard.addEffect("sound/teleport.wav");
  soundBoard.addEffect("sound/death.wav");

  level_timer = 0;
}

void Engine::newGame(bool startOver)
{
  if(startOver)
    nextMap = "start.map";
  else
  {
    // time the player took to complete the level
    scoreScreen->updateTimeScore((SDL_GetTicks() - level_timer) / 1000);
    scoreScreen->gradeTime(map.get_par_time());
    // percentage of score : total possible score
    scoreScreen->updateCompletion(100*map.get_score()/(map.updateTotalPossibleScore() + map.get_score()));
    cout << "tps : " << map.updateTotalPossibleScore() << "\n";
  }

  cout << "\n ---------------NEW GAME---------------\n";
  // free memory for any tiles and stuff
  map.unloadMap();
  //artist->clear_excess();

  // read in the new stuff from a file
  map.loadMap(nextMap);

  std::string filepath = "sprites/tileset/" + map.get_theme() + ".png";

  // get the image for the terrain
  map.set_TileSheet(artist, filepath);

  // get images for objects
  map.loadObjectImages(*artist);

  map.set_score(0);

  /// initialize the player
  player.set_image(*artist, map.get_player_type());
  player.setxSpeed(0);
  player.setySpeed(0);
  player.setHeld(false);
  gravity_timer=0;
  player.setReference(NULL);
  player.setInvuln(0);
  player.setBulletTime(0);

  player.get_bounds()->x = map.get_start().first;
  player.get_bounds()->y = map.get_start().second;

  level_timer = SDL_GetTicks();

}

/// limited to the catcher's effective radius
void Engine::player_move_held()
{
  int x, y;
  SDL_GetMouseState(&x, &y);
  x+= camera.get_bounds()->x;
  y+= camera.get_bounds()->y;
  double angle;
  SDL_Rect* cat = player.getReference()->get_bounds();
  x = (x - cat->x - cat->w/2);
  y = (y - cat->y - cat->h/2);

  angle = atan2((double)y , (double)x);

  double distance = sqrt((x*x)+(y*y));

  if( distance > CATCHER_RADIUS)
    distance = CATCHER_RADIUS;

  player.teleport(cat->x + distance * cos(angle) - 3, cat->y + distance * sin(angle) - 3);

}

void Engine::update()
{
  map.update_enemies(player.getBulletTime());
  player.use_powers();
  /// NORMAL PLAYER PHYSICS
  if(!player.getHeld())
  {
    /// movement based on gravity and current velocity
    player_move_normally();

    /// test for getting items
    // see if the player touched an item
    // the int is the point value of the item, and the string is the powerup ability
    pair<int, string> item_get = map.item_collision(player.get_bounds());
    // award points
    if(item_get.first > 0)
    {
      if(item_get.second == "N/A")
        soundBoard.playEffect("sound/points.wav");

      int score = item_get.first + map.get_score();
      map.set_score(score);
      cout << "Player's score is now " << score << ".\n";
    }
    // award powerup effects
    if(item_get.second != "N/A")
    {
      player.add_power(item_get.second);
    }
    /// test for touching enemies
    if(!player.getInvuln() && map.enemy_collision(player.get_bounds()))
    {
      soundBoard.playEffect("sound/death.wav");
      player.setInvuln(true);
      newGame();
    }
    /// test for touching teleporters
    bool reverseX = false;
    bool reverseY = false;
    if(map.tryTeleporters(player.get_bounds(), reverseX, reverseY))
      soundBoard.playEffect("sound/teleport.wav");
    if(reverseX)
      player.setxSpeed(player.getxSpeed() * -1);
    if(reverseY)
      player.setySpeed(player.getySpeed() * -1);
  }
  /// MOUSE HELD PHYSICS
  else
  {
    // stick player to the cursor or the edge of the catcher's radius
    player_move_held();
    /// test for touching undogable enemies
    if(!player.getInvuln() && map.undogable_enemy_collision(player.get_bounds()))
    {
      soundBoard.playEffect("sound/death.wav");
      player.setInvuln(true);
      newGame();
    }
  }

    // remove items marked for deletion
    map.cleanup_stuff();

  // see if player is at an exit point
  if(map.is_done(player.get_bounds()->x, player.get_bounds()->y))
  {
    soundBoard.playEffect("sound/chime.wav");
    // start the next level
    nextMap = map.get_next_map();
    newGame();
    stateMachine->setState(SCORESCREEN);
  }
}

// gravity is applied to player,
// player moves based off of velocity,
// player bounces if necessary
void Engine::player_move_normally()
{

  /// APPLY GRAVITY
  if(gravity_timer==map.get_gravity_delay())
  {
      player.changeYSpeed(1);
      gravity_timer=0;
  }
  else if(gravity_timer < map.get_gravity_delay())
      gravity_timer++;

  /// APPLY Y MOVEMENT
  //Terminal velocity for the y
  if(player.getySpeed()> TERMINAL_VELOCITY)
      player.setySpeed(TERMINAL_VELOCITY);
  else if(player.getySpeed()<-1*TERMINAL_VELOCITY)
      player.setySpeed(-1*TERMINAL_VELOCITY);

  // get the true speed
  int sp = player.getySpeed() / SPEED_MODIFIER;
  // the sign of y_i corresponds the the sign of y_speed
  int y_i = 0;

  if(sp>0)
      y_i=1;
  else if(sp<0)
      y_i=-1;
  // the direction dir corresponds to the y direction player moves
  Direction dir = DOWN;
  if(sp<0)
      dir = UP;

  // for the speed in the y direction,
  for(int i = 0; i < abs(sp); i++)
  {
      // move one pixel up or down
      player.incrementY(y_i);
      // find out if it is colliding with terrain
      // if so, get the angle of the terrain
      int collision = map.solid_collision(dir, *player.get_bounds());
      // if it is colliding with something
      if(collision)
      {
        // move backward
        player.incrementY(-1*y_i);
        // change x/y speeds to post-bounce direction
        player.bounce(map.get_angle(collision));
        // GET OUT!
        return;
      }
  }

  /// APPLY X MOVEMENT
  //Terminal velocity for the x
  if(player.getxSpeed()> TERMINAL_VELOCITY)
      player.setxSpeed(TERMINAL_VELOCITY);
  else if(player.getxSpeed()<-1*TERMINAL_VELOCITY)
      player.setxSpeed(-1*TERMINAL_VELOCITY);

  sp = player.getxSpeed() / SPEED_MODIFIER;

  // make sure we increment in the direction player is traveling
  int x_i = 0;
  dir = RIGHT;
  if(sp>0)
      x_i=1;
  else if(sp<0)
  {
      x_i=-1;
      dir = LEFT;
  }

  for(int i = 0; i < abs(sp); i++)
  {
      // move one pixel left or right
      player.incrementX(x_i);
      // find out if it is colliding with terrain
      // if so, get the angle of the terrain
      int collision = map.solid_collision(dir, *player.get_bounds());
      // if it is colliding with something
      if(collision)
      {
        // move backward
        player.incrementX(-1*x_i);
        // change x/y speeds to post-bounce direction
        player.bounce(map.get_angle(collision));
        // GET OUT!
        return;
      }
  }
}

void Engine::player_launch()
{
  // test if player is clipping into a wall
  // the direction is arbitrary in this case
  if(map.solid_collision(DOWN, *player.get_bounds()))
    return;

  // coefficient on the speed of the launcher.
  // adjust as necessary
  double launch_modifier = 1.5;
  // player bounding box
  SDL_Rect* cat = player.getReference()->get_bounds();
  // 3 is the offset from the center of the sprite to the actual catcher center.
  // The difference between the centers of player and the active catcher.
  int x = (player.getxLocation() + 3 + (player.get_bounds()->w/2) - cat->x - cat->w/2);
  int y = (player.getyLocation() + 3 + (player.get_bounds()->h/2) - cat->y - cat->h/2);

  player.setxSpeed(-1 * x * abs(launch_modifier));
  player.setySpeed(-1 * y * abs(launch_modifier));

  player.setHeld(false);
}

void Engine::drawScore()
{
  int score_length=0;
  for(int i = map.get_score(); i > 0; i/=10)
    score_length += digit_size.w;
  if(map.get_score() == 0)
    score_length = digit_size.w;
  // offset the image by its width (to the left)
  border_left.display(screen, false,  -1 * border_left.get_bounds().w, -1 * border_left.get_bounds().h + 3);
  // draw the body of the textbox
  boxColor(screen, SCORE_XPOS, SCORE_YPOS - 3 - digit_size.h, 20 + score_length + 3, SCORE_YPOS + 2, (Uint32)0x2F2F2FFF);
  // draw the right edge of the textbox
  border_right.display(screen, false, score_length, -1 * border_left.get_bounds().h + 3);
  // draw the score itself
  artist->display_numbers(map.get_score(),  SCORE_XPOS, SCORE_YPOS);
}

void Engine::display()
{
  // center the camera on the player or active catcher
  if(!player.getHeld())
    camera.center_on(*player.get_bounds());
  else
    camera.center_on(*player.getReference()->get_bounds());

  // make sure the camera isn't out of bounds
  validateCamera();

  // displays terrain tiles, background objects, items, and enemies
  map.drawMap(screen, camera.get_bounds());

  // displays the player
  player.display(*camera.get_bounds(), screen);

  // displays foreground objects
  map.drawObjects(camera.get_bounds(), screen, "foreground", false);

  drawScore();
}

void Engine::input()
{
  Uint8 *keystate = SDL_GetKeyState(NULL);
  /// KEY EVENTS - THIS IS WHEN SOMETHING IS PRESSED OR UNPRESSED (or mouse motion)
  while( SDL_PollEvent( &event ) )
  {
    // if the user clicks the X then just quit everything
    if( event.type == SDL_QUIT)
      stateMachine->setState(QUIT);
    else if( event.type == SDL_KEYDOWN )
    {
      switch(event.key.keysym.sym)
      {
        case SDLK_q: // both 'q' and 'ESC' pop us back to the main menu
        case SDLK_ESCAPE: stateMachine -> setState(MAIN_MENU); break;
        case SDLK_SPACE: newGame(); break;
        // cheat code to skip the level
        case SDLK_z: case SDLK_LCTRL: case SDLK_LSHIFT:
          if(keystate[SDLK_LCTRL] && keystate[SDLK_LSHIFT] && keystate[SDLK_z])
          {
            nextMap = map.get_next_map();
            newGame();
          }
          break;
        default: break;
      }
    }
    /// mouse button is released
    else if(event.type == SDL_MOUSEBUTTONUP)
    {
        int x, y;
        SDL_GetMouseState(&x, &y);

        // if cursor is off screen ignore it
        if(y<0 || y > WINDOW_HEIGHT || x<0 || x>WINDOW_WIDTH)
          return;

        switch(event.button.button)
        {
          case SDL_BUTTON_LEFT: // launch the player if it is currently on a catcher
                                if(player.getHeld())
                                  player_launch();
                                break;
          default: break;
        }
    }
  }
  /// KEY STATES - THIS IS WHEN SOMETHING IS CURRENTLY PRESSED
  int x, y;
  bool mouse_is_down = SDL_GetMouseState(&x, &y)&SDL_BUTTON(1);
  // this tests if the left mouse button is currently down and the player is not held by the mouse
  if(mouse_is_down  && !player.getHeld())
  {
    if(x < 0 || y < 0 || x > map.get_Widthpx() || y > map.get_Heightpx())
    {
      return;
    }
    // get refrence to list of catchers in the map
    vector<Catcher*>* catchers = map.getCatchers();
    // iterate through them
    for(vector<Catcher*>::iterator it = catchers->begin(); it != catchers->end(); it++)
    {
      Catcher* cat = *it;
      // get the coordinates of the player's center point
      int px = player.get_bounds()->x + (player.get_bounds()->w /2);
      int py = player.get_bounds()->y + (player.get_bounds()->h /2);
      // get the coordinates of the catcher's center point
      int cx = cat->get_bounds()->x + (cat->get_bounds()->w / 2);
      int cy = cat->get_bounds()->y + (cat->get_bounds()->h / 2);
      // find the distance between the points
      px = abs(px - cx);
      py = abs(py - cy);
      int difference = sqrt((px * px)+(py *py));
      // if the player is close enough to the player to catch it
      if(difference <= CATCHER_RADIUS)
      {
        // catch the player
        player.setHeld(true);
        // only player knows which catcher has caught it
        player.setReference(cat);
      }
    }
  }
  else if( !mouse_is_down && player.getHeld())
    player_launch();
}

void Engine::run()
{
  // do any post menu setup here
  stateMachine -> setState(IN_GAME);

  /// main game loop
  while( stateMachine -> getState() == IN_GAME )
  {
    // get input
    input();

    // change positions of stuff
    update();

    // draw to the screen buffer
    display();

    // displays the new screen image
    artist -> flip();

    // check how fast the last frame took to create, and delay the next one accordingly.
    // The argument specifies whether we want menu FPS or in-game FPS.
    artist -> framerate_limit(false);
  }
}

void Engine::validateCamera()
{
    // if the camera is too far left, move it
    if(camera.get_x() < 0)
        camera.set_x(0);
    // if it's too far right, move it
    else if(camera.get_x() + camera.get_w() > map.get_Widthpx())
        camera.set_x(map.get_Widthpx() - camera.get_w());
    // if it's too far up, move it
    if(camera.get_y() < 0)
        camera.set_y(0);
    // if it's too far down, move it
    else if(camera.get_y() + camera.get_h() > map.get_Heightpx())
        camera.set_y(map.get_Heightpx() - camera.get_h());
}
