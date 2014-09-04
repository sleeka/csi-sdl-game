#ifndef MENUOPTION_H_INCLUDED
#define MENUOPTION_H_INCLUDED

#include "SDL/SDL.h"
#include "Artist.h"

#include <vector>
#include <iostream>
#include <stdlib.h>
#include <time.h>

#define anim_slow 20

/***********************************************************

This can be an animated sprite, although the frames are as
tall as the image, so it's currently a one-row animation.

************************************************************/
class Image
{
  /// see the method comments in the class implementation
  public:
    Image();
    ~Image();

    void set_x(int new_x);
    void set_y(int new_y);

    void initialize(Artist* artist, std::string filename="error.png", bool semitransp = false, int x=0, int y=0);

    SDL_Rect get_bounds();

    void display(SDL_Surface* screen, bool flipped = false, int x_offset = 0, int y_offset = 0);

    void setImage(Artist& artist, std::string filename, bool flipme = false, bool magentaKey = true, bool semitransp = false);

    void frameSplice(int width, int height, bool no_repeat = true);

    void defaultFrame(bool flip);
    void setFrame(int);
    void incFrame();
    void randomFrame(int subtract);

    void setSurface(SDL_Surface* newSurf);

    int get_frames_length();

  private:
    SDL_Rect bounds;
    SDL_Surface* surface;
    SDL_Surface* horiz_flipped_surface;

    std::vector<SDL_Rect> frames;

    int frame;
};

#endif // MENUOPTION_H_INCLUDED
