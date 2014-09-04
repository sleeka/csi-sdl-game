#ifndef CATCHER_H_INCLUDED
#define CATCHER_H_INCLUDED

#include "Object.h"

using namespace std;

class Catcher : public Object
{
  public:
    Catcher(int x =0, int y=0);
    Catcher(string newType);
    Catcher(int x, int y, string newType);
    ~Catcher();
    // a lot of the stuff for this class is taken care of by Object methods
    bool initFromFile();
    int getUses();
  private:
    int uses;
};

#endif // CATCHER_H_INCLUDED
