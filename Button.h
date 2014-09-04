#ifndef BUTTON_H_INCLUDED
#define BUTTON_H_INCLUDED

#include "Image.h"

class Button
{
  public:
    Button();
    void display(SDL_Surface* screen, int x_offset = 0, int y_offset = 0);
    void initialize(Artist* artist, std::string filename="error.png", bool horiz_split=true, bool semitransp = false, int x=0, int y=0);
    void mouseover();
    void mouseoff();
    void click();
    void force_up();
    bool isDown();
    SDL_Rect get_bounds();

    void set_xOffset(int x);
    void set_yOffset(int y);

  protected:
    Image img;
    int frame;
    bool down;
};

#endif // BUTTON_H_INCLUDED
