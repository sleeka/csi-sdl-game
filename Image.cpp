#include "Image.h"

Image::Image()
{
  set_x(0);
  set_y(0);
  frame = 0;
  surface = NULL;
}

Image::~Image()
{
}

/// set offsets and load the image from file
void Image::initialize(Artist* artist, std::string filename, bool semitransp, int x, int y)
{
  set_x(x);
  set_y(y);
  surface = NULL;
  setImage(*artist, filename, false, false, semitransp);
  frameSplice(bounds.w, bounds.h);
}

/// returns an SDL_Rect representing x,y position and image dimensions
SDL_Rect Image::get_bounds()
{
  return bounds;
}

void Image::set_x(int new_x)
{
  bounds.x=new_x;
}

void Image::set_y(int new_y)
{
  bounds.y=new_y;
}

/// jump to a specific frame in the animation
void Image::setFrame(int fr)
{
  if ((fr > frames.size() - 1 ) || frames.size() <= 1)
    fr = 0;

  frame = fr*anim_slow;
}

/// procede to the next frame of the animation
/// the "times ten" thing is to slow down the animation to a tenth of the FPS
void Image::incFrame()
{
  if(++frame > anim_slow*(frames.size()) - 1 )
    frame = 0;
}

/// Display the image
void Image::display(SDL_Surface* screen, bool flipped, int x_offset, int y_offset)
{
  SDL_Rect box = bounds;
  box.x += x_offset;
  box.y += y_offset;
  SDL_Surface* sprite = surface;

  if(flipped)
    sprite = horiz_flipped_surface;

  if(frames.size() == 0)
    SDL_BlitSurface( sprite, NULL, screen, &box);
  else if(!flipped)
  {
    SDL_BlitSurface( sprite, &frames[frame/anim_slow], screen, &box);
  }
  else if(flipped)
  {
    SDL_BlitSurface( sprite, &frames[(frames.size() - 1) - (frame/anim_slow) ], screen, &box);
  }
}

/// given a new surface, ditch the current one and point to that one instead
void Image::setSurface(SDL_Surface* newSurf)
{
  if(surface != NULL)
    SDL_FreeSurface(surface);

  surface = newSurf;
}

/*********************************************************************************************************
load image from file, specify whether:
    -we should store a horizontally flipped version as well (good for things that move left and right)
    -the chroma key is mangenta (true) or black (false)
    -we should load in RGBA (true) or RGB mode with chroma key (false)
This will usually be called internally, as Image is initialized with initialize()
*********************************************************************************************************/
void Image::setImage(Artist& artist, std::string filename, bool flipme, bool magentaKey, bool semitransp)
{

  cout << "requesting image " << filename << "... ";

  string fullfilename = "sprites/" + filename;

  surface = artist.get_surface(fullfilename, magentaKey, semitransp);
  cout << "\n";

  bounds.w = artist.sprite_width( fullfilename );
  bounds.h = artist.sprite_height( fullfilename );

  if(flipme)
  {
    cout << "requesting mirror " << filename << "... ";
    fullfilename = "-" + fullfilename;
    horiz_flipped_surface = artist.get_surface(fullfilename, magentaKey, semitransp);
    cout << "\n";
  }
}

/// Reset the animation to its default frame.
/// If we decide to use an idle animation, we would need to do something else instead.
void Image::defaultFrame(bool flip)
{
  // note that frame doesn't directly correspond to the position in our frames vector
  // we divide frame by 10 to get the correct SDL_Rect, in order to slow the animation speed to a tenth of the FPS
  if(frames.size() == 4)
  {
    if(!flip)
      frame = 2*anim_slow;
    else
      frame = 1*anim_slow;
  }
  else
    if(!flip)
      frame = 1;
    else
      frame = anim_slow*(frames.size()-1);
}

/// devide the sprite into frame sections and store in the SDL_Rect vector "frames"
void Image::frameSplice(int width, int height, bool no_repeat)
{
  if(bounds.w <= 0 || bounds.h <= 0)
  {
    cout << "Cannot framesplice; invalid width or height.\n";
    return;
  }

  if(frames.size() > 0)
    frames.clear();

  if(!width)
    width = 32;

  for (int k = 0; k < bounds.h / height; k++)
  {
    for(int i = 0; i < bounds.w / width; i++)
    {
      SDL_Rect framebox;
      framebox.x = i*width;
      framebox.y = k*height;
      framebox.w = width;
      framebox.h = height;
      frames.push_back(framebox);
    }
  }

  //for 3 frame walking animations, we assume the middle should be repeated (every other frame)
  if (frames.size() == 3 && !no_repeat)
  {
    SDL_Rect copyFrame = frames[1];
    frames.push_back(copyFrame);
  }
}

int Image::get_frames_length()
{
  return frames.size();
}

void Image::randomFrame(int subtract)
{
  int max = frames.size() - subtract;
  if(max > 0)
    frame = anim_slow * (rand() % max);
}
