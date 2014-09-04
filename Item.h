#ifndef ITEM_H_INCLUDED
#define ITEM_H_INCLUDED

#include "Object.h"
#include <cstdlib>
#include <string>

using namespace std;

class Item : public Object
{
  public:
    Item(string type);
    Item(int x, int y, string type);
    int get_point_value();
    string get_power();
    int pickup();
    int pointValue();
    bool is_dead();
  protected:
    bool initFromFile();
    string power;
    int point_value;
    bool dead;

};

#endif // ITEM_H_INCLUDED
