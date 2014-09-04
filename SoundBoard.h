#ifndef SOUNDBOARD_H_INCLUDED
#define SOUNDBOARD_H_INCLUDED

#include "SDL/SDL_mixer.h"
#include <string>
#include <map>
#include <iostream>

using namespace std;

class SoundBoard
{
  public:
    SoundBoard();
    ~SoundBoard();
    void playEffect(string effect);
    void addEffect(string effect);

  private:
    map<string, Mix_Chunk*> soundEffects;

};

#endif // SOUNDBOARD_H_INCLUDED
