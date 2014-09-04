#include "Pencil.h"

Pencil::Pencil(Camera* cam, Map* m)
{
  drawing = false;
  camera = cam;
  map = m;
}

bool Pencil::isDrawing()
{
  return drawing;
}

void Pencil::removeLastCoord()
{
  if(!outline.empty())
    outline.pop_back();
}

void Pencil::toggleDrawing()
{
  drawing = !drawing;
  outline.clear();
}

void Pencil::drawOutline(SDL_Surface* screen)
{
  int curs_x = 0;
  int curs_y = 0;

  SDL_GetMouseState(&curs_x, &curs_y);

  if(curs_y > BUTTONSIZE)
  {
    curs_x += camera->get_x();
    curs_y += camera->get_y();

    if(!outline.empty() && drawing)
    {
      while(curs_x > outline[outline.size()-1].x + TILESIZE || curs_y > outline[outline.size()-1].y + TILESIZE || curs_x < outline[outline.size()-1].x - TILESIZE || curs_y < outline[outline.size()-1].y - TILESIZE)
      {
          int pointx =outline[outline.size()-1].x;
          int pointy =outline[outline.size()-1].y;

          if(curs_x > outline[outline.size()-1].x + TILESIZE)
            pointx += TILESIZE;
          else if(curs_x < outline[outline.size()-1].x - TILESIZE)
            pointx -= TILESIZE;
          if (curs_y > outline[outline.size()-1].y + TILESIZE)
            pointy += TILESIZE;
          else if (curs_y < outline[outline.size()-1].y - TILESIZE)
            pointy -= TILESIZE;

          addPoint(pointx, pointy);

      }
    }

    if(drawing)
      addPoint(curs_x, curs_y);

    Coord prevcord;

    for(int i = 0; i<outline.size(); i++)
    {
      if(i>0)
      {
        lineColor(screen, prevcord.x-camera->get_x(), prevcord.y-camera->get_y(), outline[i].x-camera->get_x(), outline[i].y-camera->get_y(), (Uint32)0xFF3333FF);
      }

      prevcord.x = outline[i].x;
      prevcord.y = outline[i].y;
    }

    if(!outline.empty() && drawing)
      lineColor( screen,
             outline[outline.size() -1].x - camera->get_x(),
             outline[outline.size() -1].y - camera->get_y(),
             curs_x - camera->get_x(),
             curs_y - camera->get_y(),
             (Uint32)0xFFFFFFFF);
  }
}

void Pencil::closeOutline()
{
  for(int i = 0; i < map->get_Tiles().size(); i++)
  {
    tileOutline(map->get_Tiles()[i]);
  }

  drawing = false;
  outline.clear();
  readyToInterpret = true;
}

/*
    Uses the even-odd rule to determine which of 8 points on a tile are inside the polygon.
    It then determines what the tile should be based on which points are inside.
    It works for 32x32 px tiles, but I haven't tried it on anything else.
*/
void Pencil::tileOutline(Tile* tile)
{
  SDL_Rect* box = tile->get_box();
  int tile_x = box->x*TILESIZE;
  int tile_y = box->y*TILESIZE;

  // 8 coordinates around the edges of the tile are determined to be either inside
  // or outside the polygon. Based off of which ones are inside, we can determine
  // what kind of shape this tile should be.
  Coord TL = {tile_x+7, tile_y+2};
  Coord TR = {tile_x+TILESIZE-7, tile_y+2};
  Coord RT = {tile_x+TILESIZE-2, tile_y+7};
  Coord RB = {tile_x+TILESIZE-2, tile_y+TILESIZE-7};
  Coord BR = {tile_x+TILESIZE-7, tile_y+TILESIZE-2};
  Coord BL = {tile_x+7, tile_y+TILESIZE-2};
  Coord LB = {tile_x+2, tile_y+TILESIZE-7};
  Coord LT = {tile_x+2, tile_y+7};

  bool bTL = pointInside(TL);
  bool bTR = pointInside(TR);
  bool bRT = pointInside(RT);
  bool bRB = pointInside(RB);
  bool bBR = pointInside(BR);
  bool bBL = pointInside(BL);
  bool bLB = pointInside(LB);
  bool bLT = pointInside(LT);

  if(bTL && bTR && bRT && bRB && bBR && bBL && bLB && bLT)
    tile->set_type(2);
  else if(bTL && bTR && bRT && !bRB && !bBR && !bBL && !bLB && bLT)
    tile->set_type(30);
  else if(!bTL && bTR && bRT && bRB && bBR && !bBL && !bLB && !bLT)
    tile->set_type(31);
  else if(bTL && !bTR && !bRT && !bRB && !bBR && bBL && bLB && bLT)
    tile->set_type(32);
  else if(!bTL && !bTR && !bRT && bRB && bBR && bBL && bLB && !bLT)
    tile->set_type(33);
  else if(!bTL && !bTR && bRT && bRB && bBR && bBL && !bLB && !bLT)
    tile->set_type(19);
  else if(bTL && bTR && !bRT && !bRB && !bBR && !bBL && bLB && bLT)
    tile->set_type(11);
  else if(bTL && bTR && bRT && bRB && !bBR && !bBL && !bLB && !bLT)
    tile->set_type(12);
  else if(!bTL && !bTR && !bRT && !bRB && bBR && bBL && bLB && bLT)
    tile->set_type(18);
  else if(!bTL && !bTR && !bRT && !bRB && bBR && bBL && bLB && !bLT)
    tile->set_type(6);
  else if(!bTL && !bTR && !bRT && bRB && bBR && bBL && !bLB && !bLT)
    tile->set_type(3);
  else if(bTL && bTR && !bRT && !bRB && !bBR && !bBL && !bLB && bLT)
    tile->set_type(27);
  else if(bTL && bTR && bRT && !bRB && !bBR && !bBL && !bLB && !bLT)
    tile->set_type(24);
  else if(!bTL && !bTR && !bRT && !bRB && !bBR && bBL && bLB && bLT)
    tile->set_type(29);
  else if(!bTL && !bTR && bRT && bRB && bBR && !bBL && !bLB && !bLT)
    tile->set_type(28);
  else if(!bTL && bTR && bRT && bRB && !bBR && !bBL && !bLB && !bLT)
    tile->set_type(49);
  else if(bTL && !bTR && !bRT && !bRB && !bBR && !bBL && bLB && bLT)
    tile->set_type(50);
  else if(!bTL && !bTR && !bRT && bRB && bBR && bBL && bLB && bLT)
    tile->set_type(5);
  else if(!bTL && !bTR && bRT && bRB && bBR && bBL && bLB && !bLT)
    tile->set_type(4);
  else if(bTL && bTR && bRT && !bRB && !bBR && !bBL && bLB && bLT)
    tile->set_type(26);
  else if(bTL && bTR && bRT && bRB && !bBR && !bBL && !bLB && bLT)
    tile->set_type(25);
  else if(bTL && !bTR && !bRT && !bRB && bBR && bBL && bLB && bLT)
    tile->set_type(36);
  else if(!bTL && bTR && bRT && bRB && bBR && bBL && !bLB && !bLT)
    tile->set_type(35);
  else if(bTL && bTR && bRT && bRB && bBR && !bBL && !bLB && !bLT)
    tile->set_type(42);
  else if(bTL && bTR && !bRT && !bRB && !bBR && bBL && bLB && bLT)
    tile->set_type(43);
}


/// Determines if a given point is within the polygon.
bool Pencil::pointInside(Coord point)
{
  //if a ray from a point crosses an odd number of lines of the polygon, it is inside
  int cross = 0;

  for(int i = 0; i<outline.size(); i++)
  {
    int k = i+1;
    if (k == outline.size())
      k = 0;

    if(crosses(point, outline[i], outline[k]))
      cross++;
  }

  if(cross%2 != 0)
    return true;

  return false;
}


/// Calculates whether a ray going left from "point" crosses over a line segment with points "start" and "end"
bool Pencil::crosses(Coord point, Coord start, Coord end)
{
  //horizontal line
  if(start.y == end.y)
  {
    return false;
  }
  //vertical line
  else if(start.x == end.x)
  {
    if(point.x > start.x)
    {
      if(start.y > end.y && point.y <= start.y && point.y>= end.y)
        return true;
      else if( end.y > start.y && point.y >= start.y && point.y<= end.y)
        return true;
    }
  }

  if((start.y > end.y && point.y <= start.y && point.y>= end.y) || ( end.y > start.y && point.y >= start.y && point.y<= end.y))
  {
    Coord rayend = point;
    rayend.x = 0;
    bool first = false;
    bool second = false;

    if(((start.x - end.x)*(point.y - start.y) - (start.y - end.y)*(point.x - start.x)) >= 0)
      first = true;
    if(((start.x - end.x)*(rayend.y - start.y) - (start.y - end.y)*(rayend.x - start.x)) >= 0)
      second = true;

    if(first != second)
      return true;
  }
  return false;
}

/// attempts to add a point to the drawing path at the position of the cursor
void Pencil::addPoint(int curs_x, int curs_y)
{
  if((curs_x  - (curs_x / (TILESIZE/2) *(TILESIZE/2))) > (((curs_x+(TILESIZE/2)) / (TILESIZE/2) *(TILESIZE/2)) - curs_x))
    curs_x += (TILESIZE/2);
  if((curs_y  - (curs_y / (TILESIZE/2) *(TILESIZE/2))) > (((curs_y+(TILESIZE/2)) / (TILESIZE/2) *(TILESIZE/2)) - curs_y))
    curs_y += (TILESIZE/2);

  // the cursor must be within a 6x6 pixel area of the point to place the point
  if(((curs_x/(TILESIZE/2))%2 || (curs_y/(TILESIZE/2))%2) && !outline.empty())
  {
    if(abs(curs_x  - (curs_x/(TILESIZE/2)*(TILESIZE/2))) > 6 || abs(curs_y  - (curs_y/(TILESIZE/2)*(TILESIZE/2))) > 6)
      return;
  }

  curs_x /= (TILESIZE/2);
  curs_y /= (TILESIZE/2);

  if((curs_x)%2 != 0 && (curs_y)%2 != 0)
    return;

  if(!outline.empty())
  {
    int prev_x = outline[outline.size() -1].x/(TILESIZE/2);
    int prev_y = outline[outline.size() -1].y/(TILESIZE/2);

    if(curs_x%2 || curs_y%2)
    {
      if((curs_x == prev_x+1 || curs_x ==prev_x-1) && (curs_y == prev_y+1 || curs_y == prev_y-1))
          return;
    }
    if((prev_x%2==0) && (prev_y%2==0))
    {
      if((curs_x == prev_x+1 || curs_x ==prev_x-1) && (curs_y == prev_y))
        return;
      if ((curs_x == prev_x) && (curs_y == prev_y+1 || curs_y == prev_y-1))
        return;
    }
  }
  else if(curs_x%2 || curs_y%2)
  {
    return;
  }

  curs_x*=(TILESIZE/2);
  curs_y*=(TILESIZE/2);

  for(int i = outline.size() - 1; i>=0; i--)
  {
    if(outline[i].x == curs_x && outline[i].y == curs_y && i != outline.size() -1 )
    {
      if(outline.size() > 2 && i == 0)
        closeOutline();
      else if(i != outline.size() - 1)
      {
        int k = i;
        i=outline.size();
        outline.resize(k);
      }
    }
  }

  if(outline.empty() || outline[outline.size()-1].x != curs_x || outline[outline.size()-1].y != curs_y)
  {
    Coord newcord;
    newcord.x = curs_x;
    newcord.y = curs_y;
    outline.push_back(newcord);
  }
}
