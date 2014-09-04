#ifndef TILESHEET_H_
#define TILESHEET_H_

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"

#include "Tile.h"
#include "Artist.h"

#include <fstream>
#include <iostream>

class TileSheet{
protected:
	SDL_Surface *tileSheetSurface;
	int width;
	int height;

public:
	TileSheet();
	~TileSheet();
	void loadSheet(Artist* artist, std::string filename);
	SDL_Surface* load_image(std::string filename);
	SDL_Surface* get_Surface();
	SDL_Rect getTileFromSheet(Tile* tile);
	SDL_Rect getBackgroundTile();
	SDL_Rect getDistanceTile();
};

#endif /* TILESHEET_H_ */
