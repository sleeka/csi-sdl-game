#ifndef TILE_H_
#define TILE_H_

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"

#include <vector>
#include <map>
#include <fstream>
#include <iostream>

#include "helpers.h"

class Tile{
  protected:
    SDL_Rect box;
    int type;

  public:
    //Initializes the variables
    Tile (int x, int y, int type);

    //Get the tile type
    int get_type();
    void set_type(int t);
    void set_x(int new_x);
    void set_y(int new_y);
    //Get the collision box
    SDL_Rect *get_box();
    void printInfo();
};


#endif /* TILE_H_ */
