#import "Soundboard.h"

SoundBoard::SoundBoard()
{
  Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 );
}

SoundBoard::~SoundBoard()
{
  for(map< string, Mix_Chunk* >::iterator it = soundEffects.begin(); it != soundEffects.end(); it++)
  {
    Mix_Chunk* chunk = (*it).second;
    if(chunk != NULL)
      Mix_FreeChunk( chunk );
  }

  Mix_CloseAudio();
}

void SoundBoard::playEffect(string filepath)
{
  map< string, Mix_Chunk* >::iterator it = soundEffects.find(filepath);
  // key is present present in map
  if( it != soundEffects.end() )
  {
    Mix_PlayChannel( -1, (*it).second, 0 );
  }
}

void SoundBoard::addEffect(string filepath)
{
  map< string, Mix_Chunk* >::iterator it = soundEffects.find(filepath);
  // key is not present in map
  if( it == soundEffects.end() )
  {
    Mix_Chunk* chunk = Mix_LoadWAV(filepath.c_str());
    it = soundEffects.begin();
    soundEffects.insert(it, pair<string, Mix_Chunk* >(filepath,chunk) );
    cout << "> SoundBoard: inserting " << filepath << " into library.\n";
  }
}
