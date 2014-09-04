#ifndef ARTIST_H_INCLUDED
#define ARTIST_H_INCLUDED

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_mixer.h"
#include <string>
#include <iostream>
#include <map>
#include <vector>

#include "helpers.h"

using namespace std;

class Artist
{
  public:
    Artist(bool& quit);
    ~Artist();

    // We want to avoid using actual black, 0x000000, as shadow should have a slight blue tint.
    // Just trust me on this. This particular shade is the same used throughout Cave Story instead of black.
    void clear( Uint32 color = 0x000016);

    void flip();

    void framerate_limit(bool menu = false);

    SDL_Surface* getScreen();

    SDL_Surface* load_image(std::string filename, Uint8 r=0xFF, Uint8 g=0, Uint8 b=0xFF);
    SDL_Surface* load_semitransp( std::string filename);

    SDL_Surface* flip_surface(SDL_Surface *surface);

    SDL_Surface* get_surface(string filepath, bool magentaKey = false, bool semitransp = false);
    int sprite_width(string filepath );
    int sprite_height(string filepath );
    void clear_excess(vector<string> used);

    void display_numbers(int num, int x_offest = 20, int y_offset = WINDOW_HEIGHT - 20, bool percentage = false);

  protected:
    SDL_Surface *swScreen;
    SDL_Surface *numbers;
    SDL_Surface *percentSign;

    Uint32 get_pixel32( SDL_Surface *surface, int x, int y );
    void put_pixel32( SDL_Surface *surface, int x, int y, Uint32 pixel );

    map<string, pair< SDL_Surface*, SDL_Surface*> > surface_list;

};

#endif // ARTIST_H_INCLUDED
