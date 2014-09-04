#ifndef Enemy_H_INCLUDED
#define Enemy_H_INCLUDED

#include "Object.h"
#include <cstdlib>
#include <string>
#include <math.h>

using namespace std;

class Enemy: public Object
{
  public:
    Enemy();
    Enemy(string type = "missing");

    ~Enemy();

    Enemy(Enemy&);

    void set_speed(int);
    void set_image(Artist& artist, std::string filepath = "");
    void set_bounds(SDL_Rect box);
    void addPoint(pair<int,int> point);
    void clone(Enemy&);
    Enemy operator=(Enemy&);

    bool isDodgable() { return dodgable; }

    void act(int slowmotion = 0);

    int get_speed();
    int get_image();

    virtual SDL_Rect* get_bounds();

    void display(SDL_Rect camera, SDL_Surface* screen);

    void teleport(SDL_Rect box);
    void teleport(int x, int y);
    void startPath();

    vector<pair<int, int> >* getPath();

  protected:
    int speed;
    pair<int,int> trigger;
    int node;
    bool loop, facing_left, forward, dodgable;

    vector<pair<int,int> > path;

    virtual bool initFromFile();
};

#endif // Enemy_H_INCLUDED
