#include "Tile.h"
#include <iostream>

Tile::Tile (int x, int y, int my_type)
{
  box.x = x;
  box.y = y;

  box.w = TILESIZE;
  box.h = TILESIZE;
	type = my_type;

}

int Tile::get_type()
{
  return type;
}

void Tile::set_type(int t)
{
	type = t;
}

SDL_Rect *Tile::get_box()
{
  return &box;
}

void Tile::set_x(int new_x)
{
  box.x=new_x;
}

void Tile::set_y(int new_y)
{
  box.y=new_y;
}

void Tile::printInfo()
{
  std::cout << box.x << ", " << box.y << " : " << type <<"\n";
}
