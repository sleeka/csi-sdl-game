#include "TileSheet.h"
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"

TileSheet::TileSheet()
{
	width = 0;
	height = 0;
	tileSheetSurface = NULL;
}

TileSheet::~TileSheet()
{
	if(tileSheetSurface)
		SDL_FreeSurface(tileSheetSurface);
}

/**
 * Load a tilesheet from a file, and tell if the file should have transparency or chroma-key.
 */
void TileSheet::loadSheet(Artist* artist, std::string filename)
{
	if(tileSheetSurface!=NULL)
	{
		std::cout << "Freeing tileSheetSurface\n";
		SDL_FreeSurface(tileSheetSurface);
	}
		std::cout << "loading tileSheetSurface from " << filename << "\n";

        tileSheetSurface = artist -> load_image(filename);

	if(tileSheetSurface)
	{
		width = tileSheetSurface->w/TILESIZE;
		height = tileSheetSurface->h/TILESIZE;
		std::cout << "Tile sheet loaded successfully from " << filename << " with dimensions: " << width << "," << height << "\n\n";
	}
	else
    {
		width = 0;
		height = 0;
		std::cout << "Tile sheet loading failed\n\n";
	}

}

/**
 * Get the SDL_Surface pointer for this tile sheet.
 */
SDL_Surface *TileSheet::get_Surface()
{
	return tileSheetSurface;
}

/**
 * Get the location on the tilesheet of a certain tile based on its subtype
 */
SDL_Rect TileSheet::getTileFromSheet(Tile* tile)
{
	SDL_Rect ret;
		ret.x = tile->get_type()%width*TILESIZE;
		ret.y = tile->get_type()/width*TILESIZE;
        ret.h = TILESIZE;
        ret.w = TILESIZE;
	return ret;
}

SDL_Rect TileSheet::getBackgroundTile()
{
	SDL_Rect ret;

	ret.x = tileSheetSurface->w-64;
	ret.y = tileSheetSurface->h-64;
	ret.h = 64;
	ret.w = 64;
	return ret;
}

SDL_Rect TileSheet::getDistanceTile()
{
	SDL_Rect ret;

	ret.x = tileSheetSurface->w-128;
	ret.y = tileSheetSurface->h-64;
	ret.h = 64;
	ret.w = 64;
	return ret;
}
