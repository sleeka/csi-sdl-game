#ifndef PENCIL_H_INCLUDED
/************************************************
  Pencil is used to create terrain in the editor
by drawing an outline of a closed polygon.
  Originally all this code was in Editor, and was
refactored into a seperate class mostly for
readability.
************************************************/
#define PENCIL_H_INCLUDED

#include "SDL/SDL_gfxPrimitives.h"

#include "helpers.h"
#include "Camera.h"
#include "Map.h"

class Pencil
{
  public:
    Pencil(Camera* cam, Map* map);
    Camera* camera;
    Map* map;

    // editing and displaying the outline
    void drawOutline(SDL_Surface* screen);
    void removeLastCoord();
    void toggleDrawing();

    // drawing getter
    bool isDrawing();
    // Yes this is a public boolean.
    // It is read once, and modified
    // once in Editor and once in Pencil
    bool readyToInterpret;

  private:
    bool drawing;
    vector<Coord > outline;

    // forming the outline
    void addPoint(int curs_x, int curs_y);
    void closeOutline();

    // conversion into tiles
    void interpretPoints(Direction& dir, int place);
    void tileOutline(Tile* tile);
    bool crosses(Coord point, Coord start, Coord end);
    bool pointInside(Coord point);
};

#endif // PENCIL_H_INCLUDED
