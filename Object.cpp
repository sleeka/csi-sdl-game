#include "Object.h"
using namespace std;

Object::Object(int x, int y)
{
  bounds.x = x;
  bounds.y = y;
  scrollspeed = 1;
  successful_load = false;
}

Object::Object(string newType)
{
  bounds.x = 0;
  bounds.y = 0;
  type = newType;
  scrollspeed = 1;

  successful_load = initFromFile();
}

Object::Object(int x, int y, string newType)
{
  bounds.x = x;
  bounds.y = y;
  type = newType;
  scrollspeed = 1;
  successful_load = initFromFile();
}

Object::~Object()
{
}

// to tell Engine to remove this one if it didn't load right
bool Object::load_successful()
{
  return successful_load;
}

string Object::get_type()
{
  return type;
}

SDL_Rect* Object::get_bounds()
{
  return &bounds;
}

void Object::set_image(Artist& artist, string filepath)
{
  if(filepath == "" && type!="")
      filepath = "object/" + type + ".png";

  img.setImage(artist, filepath, false, true);

  //assumes animations are horizontal
  img.frameSplice(bounds.h, bounds.h);
}

int Object::get_scrollspeed()
{
  return scrollspeed;
}

void Object::display(SDL_Rect camera, SDL_Surface* screen, bool lock_position)
{
  img.incFrame();

  if(scrollspeed == 1 || lock_position)
    img.display(screen, false, bounds.x - camera.x, bounds.y - camera.y);
  else if(scrollspeed == 3)
    img.display(screen, false, bounds.x - camera.x + (camera.x*2/3), bounds.y - camera.y + (camera.y*2/3));
  else if(scrollspeed == 4)
    img.display(screen, false, bounds.x - camera.x + (camera.x*3/4), bounds.y - camera.y + (camera.y*3/4));
}

// obviously this will change depending on what we what we need
bool Object::initFromFile()
{
  string filepath = "data/objects/" + type + ".obj";

	cout << "Loading object " << filepath << ", ";

	ifstream objectFile(filepath.c_str(), std::ifstream::in);

	if(objectFile != NULL)
	{
		cout << "ifstream successfull... ";

    string tag = "";

    while(getline(objectFile, tag))
    {
        if(tag == "SIZE")
        {
          objectFile >> bounds.h;
          bounds.w = bounds.h;
          cout << "size = [" << bounds.w << " x " << bounds.h << "] ";
        }
        else if(tag == "ANCHOR" || tag=="ANCHORED")
        {
          string anch;
          objectFile >> anch;
          if( anch == "distance" || anch == "Distance" || anch == "distant" || anch == "Distant")
          {
              scrollspeed = 4;
          }
          else if(anch == "background" || anch == "Background")
          {
              scrollspeed = 3;
          }
          else
          {
              scrollspeed = 1;
          }
          cout << "scrollspeed = [" << scrollspeed << "] ";
        }
        else if (tag[0] = '#' || tag == "" || tag == " "){
        }else
          cout << "skipping unknown tag \"" << tag << "\" ";
    }
    cout << "\n";
    return true;
	}

	cout << "invalide input file.\n";
	return false;
}
