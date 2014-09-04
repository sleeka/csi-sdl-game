#include "Teleporter.h"

using namespace std;

Teleporter::Teleporter(Coord A, Coord B, Coord end)
{
  gateA = A;
  gateB = B;
  reverseX = false;
  reverseY = false;
  if(end.x < 0)
  {
    reverseY = true;
    end.x = abs(end.x);
  }
  if(end.y < 0)
  {
    reverseX = true;
    end.y = abs(end.y);
  }

  line_end = end;
  cooldown = 0;
}

Coord Teleporter::getA()
{
  return gateA;
}

Coord Teleporter::getB()
{
  return gateB;
}

Coord Teleporter::getEnd()
{
  return line_end;
}

bool Teleporter::reverseXDirection()
{
  return reverseX;
}

bool Teleporter::reverseYDirection()
{
  return reverseY;
}

bool Teleporter::teleport(SDL_Rect* box)
{
  // vertical teleporter
  if(line_end.x == 0)
  {
    if(box->x <= gateA.x && box->x+box->w >= gateA.x && box->y <= gateA.y + line_end.y && box->y+box->h >= gateA.y)
    {
      if(!cooldown)
      {
        cout << " [EVENT] teleported from " << box->x << ", " << box->y;
        box->x = gateB.x - ( gateA.x - box->x );
        box->y = gateB.y - ( gateA.y - box->y );
        cout << " to " << box->x << ", " << box->y << "\n";
        cooldown = 2;
        return true;
      }
    }
    else if(box->x <= gateB.x && box->x+box->w >= gateB.x && box->y <= gateB.y + line_end.y && box->y+box->h >= gateB.y)
    {
      if(!cooldown)
      {
        cout << " [EVENT] teleported from " << box->x << ", " << box->y;
        box->x = gateA.x - ( gateB.x - box->x );
        box->y = gateA.y - ( gateB.y - box->y );
        cout << " to " << box->x << ", " << box->y << "\n";
        cooldown = 2;
        return true;
      }
    }
    // finally got away from the teleporter
    else if(cooldown>0)
    {
      cooldown--;
    }
  }
  // horizontal teleporter
  else if(line_end.y == 0)
  {
    if(box->y <= gateA.y && box->y+box->h >= gateA.y && box->x + box->w >= gateA.x && box->x <= gateA.x + line_end.x)
    {
      if(!cooldown)
      {
        cout << " [EVENT] teleported from " << box->x << ", " << box->y;
        box->x = gateB.x - ( gateA.x - box->x );
        box->y = gateB.y - ( gateA.y - box->y );
        cout << " to " << box->x << ", " << box->y << "\n";
        cooldown = 2;
        return true;
      }
    }
    else if (box->y <= gateB.y && box->y+box->h >= gateB.y && box->x + box->w >= gateB.x && box->x <= gateB.x + line_end.x)
    {
      if(!cooldown)
      {
        cout << " [EVENT] teleported from " << box->x << ", " << box->y;
        box->x = gateA.x - ( gateB.x - box->x );
        box->y = gateA.y - ( gateB.y - box->y );
        cout << " to " << box->x << ", " << box->y << "\n";
        cooldown = 2;
        return true;
      }
    }
    // finally got away from the teleporter
    else if(cooldown>0)
    {
      cooldown--;
    }
  }
  return false;
}
