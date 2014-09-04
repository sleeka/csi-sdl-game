#ifndef TELEPORTER_H_INCLUDED
#define TELEPORTER_H_INCLUDED

#include <iostream>

#include "SDL/SDL.h"
#include "helpers.h"

// This is more of a "stargate" and consists of two lines.
// Touching one line teleports a rect to the other
class Teleporter
{
  public:
    Teleporter(Coord A, Coord B, Coord end);

    bool teleport(SDL_Rect* box);
    bool reverseXDirection();
    bool reverseYDirection();

    Coord getA();
    Coord getB();
    Coord getEnd();

  protected:
    Coord gateA;
    Coord gateB;
    Coord line_end;
    int cooldown;
    bool reverseX, reverseY;
};

#endif // TELEPORTER_H_INCLUDED
