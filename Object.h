#ifndef OBJECT_H_INCLUDED
#define OBJECT_H_INCLUDED

#include <fstream>

#include "SDL/SDL.h"

#include "Image.h"
#include "helpers.h"
#include "Artist.h"

using namespace std;

class Object
{
  public:
    Object(int x =0, int y=0);
    Object(string newType);
    Object(int x, int y, string newType);
    virtual ~Object();

    virtual SDL_Rect* get_bounds();
    virtual void set_image(Artist& artist, string filepath = "");
    virtual void display(SDL_Rect camera, SDL_Surface* screen, bool lock_position = false);
    string get_type();
    int get_scrollspeed();

    bool load_successful();

  protected:
    // true if object is in the distance, and should be scrolled more slowly
    bool successful_load;
    int scrollspeed;

    string type;

    int xVelocity, yVelocity, damage;

    SDL_Rect bounds;

    Image img;

    virtual bool initFromFile();
};

#endif // OBJECT_H_INCLUDED
