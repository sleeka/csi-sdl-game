#include "Artist.h"

using namespace std;

Artist::Artist(bool& quit)
{
  SDL_Init( SDL_INIT_EVERYTHING );
  SDL_WM_SetCaption( "Animal Adventure v1.0.0", NULL );
  //SDL_ShowCursor(SDL_DISABLE);
  swScreen = SDL_SetVideoMode( WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_BPP, SDL_SWSURFACE);
  numbers = get_surface("sprites/numbers.png", true, false);
  percentSign = get_surface("sprites/percent.png", true, false);

  if(!numbers)
    cout << "> Artist: could not load the sprites/numbers.png\n";
  // if we failed to create the screen surface
  if(swScreen==NULL)
  {
      std::cout << "screen is still NULL\n";
      quit=true;
  }

  Uint8 *SDL_GetKeyState(int *numkeys);
}

Artist::~Artist()
{
  if(swScreen!=NULL)
      SDL_FreeSurface(swScreen);

  // apparently this part segfaults
  map<string, pair< SDL_Surface*, SDL_Surface*> >::iterator it = surface_list.begin();
  for(; it!=surface_list.end(); it++)
  {
    SDL_Surface* normal = (*it).second.first;
    SDL_Surface* mirror = (*it).second.second;

    if(normal != NULL)
      SDL_FreeSurface(normal);
    if(mirror != NULL)
      SDL_FreeSurface(mirror);
  }
  surface_list.clear();

  SDL_Quit();
}

///returns a pointer to the screen surface so we can blit to it
SDL_Surface* Artist::getScreen()
{
  // "software screen" - we are not using the video card
  return swScreen;
}

/// draws a rectangle of color "color" over the entire screen
void Artist::clear(Uint32 color)
{
  SDL_FillRect( swScreen, NULL, color);
}

SDL_Surface* Artist::get_surface(string filepath, bool magentaKey, bool semitransp)
{
  bool flipped = false;
  if(filepath[0] == '-')
  {
    //cout << "set to flip; ";
    filepath = filepath.substr(1);
    flipped = true;
  }

  map< string, pair< SDL_Surface*, SDL_Surface* > >::iterator it = surface_list.find(filepath);
  // key is already present in map
  if( it != surface_list.end() )
  {
    // if normal is wanted and available
    if(!flipped && (*it).second.first)
    {
      cout << "already loaded.";
      return (*it).second.first;
    }
    // if reverse is wanted and available
    else if(flipped && (*it).second.second)
    {
      cout << "already loaded.";
      return (*it).second.second;
    }
    else if(flipped && (*it).second.first)
    {
        (*it).second.second = flip_surface((*it).second.first);
        if ((*it).second.second )
        {
          cout << "successfully generated.";
          return (*it).second.second;
        }
        else
        {
          cout << "failed to flip image.";
          return 0;
        }
    }
  }
  // key is not in map yet
  else
  {
    SDL_Surface* surface;

    if(semitransp)
      surface = load_semitransp(filepath);
    else if(!magentaKey)
      surface = load_image(filepath, 0, 0, 0); //color key black
    else
      surface = load_image(filepath);

    if(surface)
    {
      cout << "successfully created.";
      pair< SDL_Surface*, SDL_Surface*> surf_pair;
      surf_pair.first = surface;
      SDL_Surface* flip;

      if(flipped)
      {
          flip = flip_surface(surface);
          surf_pair.second = flip;
          if (flip)
            cout << "successfully generated.";
          else
            cout << "failed to flip image.";
      }

      it = surface_list.begin();
      surface_list.insert(it, pair<string, pair< SDL_Surface*, SDL_Surface* > >(filepath,surf_pair) );

      // if they wanted the normal version
      if(!flipped)
        return surf_pair.first;
      // if it wanted the reverse, and reverse is available
      else  if(flipped && surf_pair.second)
        return surf_pair.second;
    }
    else
    {
        cout << "loading failed\n";
        return NULL;
    }
  }
  cout << "loading failed\n";
  return NULL;
}

void Artist::clear_excess(vector<string> used)
{
  map<string, pair< SDL_Surface*, SDL_Surface*> >::iterator it = surface_list.begin();
  for(; it!=surface_list.end(); it++)
  {
    string search = (*it).first;
    if( ((*it).first.substr(0, 14) == "sprites/enemy/" || (*it).first.substr(0, 15) == "sprites/object/"))
    {
      // TODO: loop through vector

      SDL_Surface* normal = (*it).second.first;
      SDL_Surface* mirror = (*it).second.second;

      if(normal != NULL)
        SDL_FreeSurface(normal);
      if(mirror != NULL)
        SDL_FreeSurface(mirror);
    }
  }
}

int Artist::sprite_width(string filepath)
{
  map< string, pair< SDL_Surface*, SDL_Surface* > >::iterator it = surface_list.find(filepath);
  if( it == surface_list.end())
  {
    cout << filepath << "does not exist in surface list!\n";
    return 0;
  }
  if(!(*it).second.first)
  {
    cout << filepath << " has no associated surface!\n";
    return 0;
  }

  if((*it).second.first->w > 0)
    return (*it).second.first->w;

  cout << filepath << " has invalide width!\n";
  return 0;
}

int Artist::sprite_height(string filepath)
{
  map< string, pair< SDL_Surface*, SDL_Surface* > >::iterator it = surface_list.find(filepath);
  if( it == surface_list.end())
  {
    cout << filepath << "does not exist in surface list!\n";
    return 0;
  }
  if(!(*it).second.first)
  {
    cout << filepath << " has no associated surface!\n";
    return 0;
  }

  if((*it).second.first->h > 0)
    return (*it).second.first->h;

  cout << filepath << " has invalide height!\n";
  return 0;
}

// displays numbers, probably used for the score
// offsets refer to the LOWER LEFT corner of the text field
void Artist::display_numbers(int num, int x_offset, int y_offset, bool percentage)
{
  int frameheight = sprite_height("sprites/numbers.png");
  int remainder;
  SDL_Rect box;
  box.x = x_offset;
  box.y = y_offset - frameheight;
  if(num == 0)
  {
    SDL_Rect frame;
    frame.y = 0;
    frame.x = 0;
    frame.w = frameheight;
    frame.h = frameheight;
    SDL_BlitSurface( numbers, &frame, swScreen, &box);
    return;
  }
  int digit = 0;
  for(int i = num; i > 0; i/=10)
  {
    digit++;
  }
  if(percentage)
  {
    box.x = x_offset + (digit)*frameheight + 5;
    SDL_BlitSurface( percentSign, NULL, swScreen, &box);
  }

  while( num > 0)
  {
    digit--;
    remainder = num % 10;
    num /= 10;
    box.x = x_offset + digit*frameheight;
    SDL_Rect frame;
    frame.y = 0;
    frame.x = remainder*frameheight;
    frame.w = frameheight;
    frame.h = frameheight;
    SDL_BlitSurface( numbers, &frame, swScreen, &box);
  }
}

/// return the color of a specific pixel on a surface
Uint32 Artist::get_pixel32(SDL_Surface *surface, int x, int y)
{
  //Convert the pixels to 32 bit
  Uint32 *pixels = (Uint32 *)surface->pixels;

  //Get the requested pixel
  return pixels[ ( y * surface->w ) + x ];
}

/// set the color of a specific pixel on a surface
void Artist::put_pixel32(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
  //Convert the pixels to 32 bit
  Uint32 *pixels = (Uint32 *)surface->pixels;

  //Set the pixel
  pixels[ ( y * surface->w ) + x ] = pixel;
}

/// return a new surface that is a flipped version of a provided surface
// assumes horizontal flip
SDL_Surface* Artist::flip_surface(SDL_Surface *surface)
{
  // pointer to the new (flipped) surface
  SDL_Surface *flipped = NULL;

  // if the image is color keyed
  if( surface->flags & SDL_SRCCOLORKEY )
  {
    flipped = SDL_CreateRGBSurface( SDL_SWSURFACE, surface->w, surface->h, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, 0 );
  }
  else
  {
    flipped = SDL_CreateRGBSurface( SDL_SWSURFACE, surface->w, surface->h, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask );
  }

  // lock the surface if we have to
  if( SDL_MUSTLOCK( surface ) )
  {
    SDL_LockSurface( surface );
  }

  // iterate through the columns
  for( int x = 0, rx = flipped->w - 1; x < flipped->w; x++, rx-- )
  {
    // iterate through the rows
    for( int y = 0, ry = flipped->h - 1; y < flipped->h; y++, ry-- )
    {
      // get pixel
      Uint32 pixel = get_pixel32( surface, x, y );
      // copy pixel
      put_pixel32( flipped, rx, y, pixel );
    }
  }

  // unlock surface
  if( SDL_MUSTLOCK( surface ) )
  {
    SDL_UnlockSurface( surface );
  }

  // copy color key
  if( surface->flags & SDL_SRCCOLORKEY )
  {
    SDL_SetColorKey( flipped, SDL_RLEACCEL | SDL_SRCCOLORKEY, surface->format->colorkey );
  }

  // return flipped surface
  return flipped;
}

void Artist::flip()
{
	// update screen
  SDL_Flip( swScreen );
}

void Artist::framerate_limit(bool menu)
{
  int fps = (menu ? MENU_FRAMES_PER_SECOND : FRAMES_PER_SECOND);

  static int lastTime = 0;
  int timeSpent = SDL_GetTicks() - lastTime;

  if( timeSpent < 1000 / fps )
    SDL_Delay( ( 1000 / fps ) - timeSpent);
}

SDL_Surface* Artist::load_image(std::string filename, Uint8 r, Uint8 g, Uint8 b)
{
  // the image that's loaded
  SDL_Surface* loadedImage = NULL;

  // if we can optimize it, we will return that instead
  SDL_Surface* optimizedImage = NULL;

  // load the image
  loadedImage = IMG_Load( filename.c_str() );

  // if the image successfully loaded
  if( loadedImage != NULL )
  {
    // create an optimized surface
    optimizedImage = SDL_DisplayFormat( loadedImage );

    // if the surface was optimized
    if( optimizedImage != NULL )
    {
      // free the old surface (never use "delete" on an SDL_Surface pointer)
      SDL_FreeSurface( loadedImage );

      // color key surface
      SDL_SetColorKey( optimizedImage, SDL_SRCCOLORKEY, SDL_MapRGB( optimizedImage->format, r, g, b ) );
    }
    else
    {
      // well we still have the original surface.
      return loadedImage;
    }

  }
  else
  {
    //aw crap.
    //TODO: handle this correctly
  }
  return optimizedImage;
}

SDL_Surface* Artist::load_semitransp( std::string filename)
{
  SDL_Surface* loadedImage = NULL;
  loadedImage = IMG_Load( filename.c_str() );
  // don't even try to optimize it; we want RGBA values.
  // I think there is a way to optimize while preserving alpha though
  return loadedImage;
}
