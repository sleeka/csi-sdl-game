#include "player.h"
#include <iostream>

using namespace std;

Player :: Player()
{
	bounds.x = 0;
	bounds.y = 0;
	x_speed = 0;
	y_speed = 0;
	held = false;
	ref = NULL;
	invuln = 0;
	facing_left = false;
	soundBoard = NULL;
}

Player :: ~Player()
{
}

void Player :: changeYSpeed(int ys)
{
    y_speed += ys;
}

int Player :: getxLocation()
{
	return bounds.x;
}

int Player :: getyLocation()
{
	return bounds.y;
}

void Player::use_powers()
{
  if(invuln > 0)
  {
    invuln--;
    if(!invuln)
    {
      cout << " [EVENT] : invuln has worn off.\n";
      soundBoard->playEffect("sound/invuln_off.wav");
      img.randomFrame(1);
    }
  }
  if(bullettime > 0)
  {
    bullettime--;
    if(!bullettime)
      cout << " [EVENT] : Time is now back to normal.\n";
  }

  for(int i = 0; i<powers.size(); i++)
  {
    if(powers[i] == "INVULN")
    {
      cout << " [EVENT] : Invulnerability gained!\n";
      invuln = 1000;
      img.setFrame((img.get_frames_length()-1));
      powers.erase(powers.begin() + i);
      soundBoard->playEffect("sound/invuln.wav");
      i--;
    }
    else if(powers[i] == "SLOWTIME")
    {
      cout << " [EVENT] : Time has been slowed!\n";
      bullettime = 1000;
      powers.erase(powers.begin() + i);
      soundBoard->playEffect("sound/slowtime.wav");
      i--;
    }
  }
}

void Player::setSoundBoard(SoundBoard* sound)
{
  soundBoard = sound;
}

void Player::add_power(string power)
{
  powers.push_back(power);
}

void Player::display(SDL_Rect camera, SDL_Surface* screen)
{
  img.display(screen, facing_left, bounds.x - camera.x, bounds.y - camera.y);
}

void Player::set_image(Artist& artist, string filepath)
{
  if(filepath == "")
    filepath = "bug";

  filepath = "player/" + filepath + ".png";
  img.setImage(artist, filepath, true, true);

  bounds.w = img.get_bounds().h;
  bounds.h = img.get_bounds().h;

  img.frameSplice(bounds.w, bounds.h, true);
  img.setFrame(0);
}

void Player:: setxSpeed(int xspeed)
{
	x_speed = xspeed;
}

void Player :: setySpeed(int yspeed)
{
	y_speed = yspeed;
}

void Player :: setHeld(bool h)
{
  held = h;
}

int Player :: getxSpeed()
{
	return x_speed;
}

int Player :: getySpeed()
{
	return y_speed;
}

bool Player :: getHeld()
{
  return held;
}

void Player :: incrementX(int x)
{
  bounds.x += x;
}

void Player :: incrementY(int y)
{
  //cout << "inc " << y << ", was "<< bounds.y<<"; now ";
  bounds.y += y;
  //cout << " "<< bounds.y << "y\n";
}

void Player :: bounce(double angle, double elasticity)
{
  int orig_x_speed = x_speed;

  ///to work with first quadrant vectors
  y_speed *= -1;

  //cout << "starting bounce. Ramp angle is " << angle << " degrees.\nX speed: " << x_speed << ", Y speed (flipped): " << y_speed << "\n";
  ///get (degree) angle of normal vector
  angle += 90;
  while(angle > 360)
    angle -= 360;

  //cout << "angle of normal: " << angle << " , ";
  ///convert to radians
  angle = angle * PI / 180;
  /// get the normal vector from the tile
  pair< double , double > tile_normal;
  tile_normal.first = cos(angle);
  tile_normal.second = sin(angle);
  //cout << "cos of normal: " << tile_normal.first << " , sin of normal: " << tile_normal.second << "\n";

  /// component vector of player's motion parallel to the wall's normal vector
  pair< double, double > parallel_component;
  parallel_component.first  = -1 * tile_normal.first  * abs(tile_normal.first * x_speed + tile_normal.second * y_speed);
  parallel_component.second = -1 * tile_normal.second * abs(tile_normal.first * x_speed + tile_normal.second * y_speed);
  //cout << "U's x component: " << parallel_component.first << " , U's y component: " << parallel_component.second << "\n";

  /// component vector of player's motion perpandicular to the wall's normal vector
  pair< double, double > normal_component;
  normal_component.first  = x_speed - parallel_component.first;
  normal_component.second = y_speed - parallel_component.second;
  //cout << "W's x component: " << normal_component.first << " , W's y component: " << normal_component.second << "\n";

  x_speed = normal_component.first  - elasticity*parallel_component.first;
  y_speed = normal_component.second - elasticity*parallel_component.second;

  y_speed *= -1;
  //cout << "finished bounce. X speed: " << x_speed << ", Y speed (actuall): " << y_speed << "\n";
  /// set player's sprite to random frame
  if(!invuln)
    img.randomFrame(1);

  soundBoard->playEffect("sound/bounce.wav");

  /// make sure the flipped sprite will be used if going left
  if(orig_x_speed >= 0 && x_speed < 0)
    facing_left = true;
  else if(orig_x_speed <= 0 && x_speed >0)
    facing_left = false;
}

// returns a poitner that player stores, locating the active catcher
Catcher* Player:: getReference()
{
  return ref;
}

// sets a pointer that locates the active catcher
void Player::setReference(Catcher* reference)
{
  ref = reference;
}

void Player::teleport(int x, int y)
{
  bounds.x = x;
  bounds.y = y;
}
