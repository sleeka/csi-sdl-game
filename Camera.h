#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include "SDL/SDL.h"
#include "helpers.h"

class Camera
{
  private:
    SDL_Rect bounds;
  public:
    Camera();
    Camera(SDL_Rect rect);
    SDL_Rect *get_bounds();
    void center_on(SDL_Rect box);
    int get_x();
    int get_y();
    int get_w();
    int get_h();
    void set_x(int x);
    void set_y(int y);
    void scroll(Direction dir, int dist);
};

#endif // CAMERA_H_INCLUDED
