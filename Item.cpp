#include "Item.h"

using namespace std;

Item::Item(string newType)
{
  type = newType;
  scrollspeed = 1;
  successful_load = initFromFile();
  dead = false;
}

Item::Item(int x, int y, string newType)
{
  bounds.x = x;
  bounds.y = y;
  type = newType;
  scrollspeed = 1;
  successful_load = initFromFile();
  dead = false;
}

string Item::get_power()
{
  return power;
}

int Item::pickup()
{
  dead = true;
  return point_value;
}

int Item::pointValue()
{
    return point_value;
}

bool Item::is_dead()
{
  return dead;
}

// obviously this will change depending on what we what we need
bool Item::initFromFile()
{
  string filepath = "data/objects/" + type + ".item";

	cout << "Loading item " << filepath << ", ";

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
        else if(tag == "POWER")
        {
          objectFile >> power;
          cout << "power = [" << power << "] ";
        }
        else if(tag == "VALUE")
        {
          objectFile >> point_value;
          cout << "point_value = [" << point_value << "] ";
        }
        else if (tag[0] = '#' || tag == "" || tag == " "){
        }else
          cout << "skipping unknown tag \"" << tag << "\"";
    }
    cout << "\n";
    return true;
	}
	cout << "invalide input file.\n";
	return false;
}
