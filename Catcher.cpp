#include "Catcher.h"

Catcher::Catcher(int x, int y) : Object(x,y)
{
  type = "CATCHER_blue";
  scrollspeed = 1;
}
// we don't want to use Object::Object() as it will
// call its own version of initFromeFile
Catcher::Catcher(string newType)
{
  bounds.x = 0;
  bounds.y = 0;
  type = newType;
  successful_load = Catcher::initFromFile();
  scrollspeed = 1;
}
// we don't want to use Object::Object() as it will
// call its own version of initFromeFile
Catcher::Catcher(int x, int y, string newType)
{
  bounds.x = x;
  bounds.y = y;
  type = newType;
  scrollspeed = 1;
  successful_load = Catcher::initFromFile();
}
Catcher::~Catcher()
{

}
int Catcher::getUses()
{
  return uses;
}
// obviously this will change depending on what we what we need
bool Catcher::initFromFile()
{
  string filepath = "data/objects/" + type + ".ct";

	cout << "Loading catcher " << filepath << ", ";

	ifstream objectFile(filepath.c_str(), std::ifstream::in);

	if(objectFile != NULL)
	{
		cout << "ifstream successfull... ";

    string tag = "";

    while(getline(objectFile, tag))
    {
        if(tag == "SIZE")
        {
          objectFile >> bounds.w;
          bounds.h = bounds.w;
          cout << "size = [" << bounds.w << " x " << bounds.h << "] ";
        }
        else if(tag == "USES")
        {
          objectFile >> uses;
          cout << "# uses = [" << uses << "] ";
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
