#include <iostream>
#include <cmath>

#include "Object.h"
#include "Catcher.h"
#include "SoundBoard.h"

using namespace std;

class Player : public Object
{
private:
	int x_speed;
	int y_speed;
	// timers for the powerup states
	int invuln;
	int bullettime;
  // whether the player is held by a catcher
	bool held;
	// used to display the flipped sprite
	bool facing_left;
	// contains the powers player currently has (from powerups)
	vector<string> powers;
	// the catcher that the player is held by
  Catcher* ref;
  SoundBoard* soundBoard;

public:
  Player();
  ~Player();

  // getters
  int getxLocation();
  int getyLocation();
  int getxSpeed();
  int getySpeed();
  int getInvuln(){ return invuln; }
  int getBulletTime(){ return bullettime; }
  bool getHeld();
  Catcher* getReference();

  // setters
  void setSoundBoard(SoundBoard* sound);
  void setxSpeed(int);
  void setySpeed(int);
  void setHeld(bool);
  void changeYSpeed(int);
  void incrementX(int);
  void incrementY(int);
  void teleport(int x, int y);
  void setInvuln(int i){ invuln = i;}
  void setBulletTime(int i){bullettime = i;}
  void setReference(Catcher*);
  void add_power(string power);
  void set_image(Artist& artist, string filepath="bug");

  // other
  void display(SDL_Rect, SDL_Surface*);
  void bounce(double angle, double elasticity = 1.01);
  void use_powers();
};
