#include "Button.h"

Button::Button()
{
  down = false;
  frame = 0;
}

void Button::display(SDL_Surface* screen, int x_offset, int y_offset)
{
  img.setFrame(frame);
  img.display(screen, false, x_offset, y_offset);
}

void Button::initialize(Artist* artist, std::string filename, bool horiz_split, bool semitransp, int x, int y)
{
  img.initialize(artist, filename, semitransp, x, y);
  int h = img.get_bounds().h;

  // dropdown menu buttons are wide, and use vertical frames
  // while menubar buttons are square, and use horizontal frames
  if(!horiz_split)
    img.frameSplice(img.get_bounds().w,h/2);
  else
    img.frameSplice(h,h);
}

SDL_Rect Button::get_bounds()
{
  return img.get_bounds();
}

void Button::set_xOffset(int x)
{
  img.set_x(x);
}

void Button::set_yOffset(int y)
{
  img.set_y(y);
}

void Button::mouseover()
{
  if(!down)
    frame=1;
}

void Button::mouseoff()
{
  if(down)
    frame =2;
  else
    frame =0;
}

void Button::click()
{
  down = true;
  frame = 2;

}

void Button::force_up()
{
  down = false;
  frame = 0;
}

bool Button::isDown()
{
  return down;
}
