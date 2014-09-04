#ifndef MAP_H_INCLUDED
#define MAP_H_INCLUDED

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"

#include "helpers.h"
#include "Tile.h"
#include "TileSheet.h"
#include "Enemy.h"
#include "Catcher.h"
#include "Item.h"
#include "Teleporter.h"

using namespace std;

class Map
{
  private:
    string mapFileLocation;
    vector<Tile*> tiles;
    vector<Enemy*> enemies;
    vector<Enemy*> undogableEnemies;
    vector<Catcher*> catchers;
    vector<Item*> items;
    vector<Teleporter*> teleporters;
    map< string, vector<Object*> > objectLayers;

    // in tiles (?)
    int width, height;
    int gravity_delay;
    int par_time;

    string theme;
    string next_map;

    pair<int, int> startLoc;

    TileSheet *tileSheet;

    SDL_Rect getTileScreenCoord(Tile* tile, SDL_Rect* camera);
    void blitTile(SDL_Surface* screen, SDL_Rect* camera, Tile* tile);
    bool finished;

    int score;
    string player_type;

  public:
    Map();
    ~Map();

    vector<Tile*> get_Tiles();
    vector<Tile*>* get_TilesPointer();

    Tile* get_Tile(int x, int y);
    Tile* get_Tilepx(int x, int y);
    int get_Width();
    int get_Height();
    int get_Widthpx();
    int get_Heightpx();
    int get_score();
    int get_par_time();
    pair<int, int> get_start();
    string get_theme();
    string get_player_type();
    string get_next_map();

    vector<Object*>* getObjects(string);
    vector<Enemy*>* getEnemies();
    vector<Catcher*>* getCatchers();
    vector<Item*>* getItems();
    vector<Teleporter*>* getTeleporters();

    void set_score(int s);
    void set_start(pair<int, int>);
    void set_theme(string th);
    void set_Width(int newWidth);
    void set_Height(int newHeight);
    void set_TileSheet(Artist* artist, string filename);

    int get_gravity_delay();

    void loadObjectImages(Artist& artist);

    void update_enemies(int bullettime = 0);

    void drawMap(SDL_Surface* screen, SDL_Rect* camera);
    void drawDistance(SDL_Rect* camera, SDL_Surface* screen);
    void drawBackground(SDL_Rect* camera, SDL_Surface* screen);
    void drawTiles(SDL_Rect* camera, SDL_Surface* screen);
    void drawObjects(SDL_Rect* camera, SDL_Surface* screen, string layer = "background", bool lock_position = false);

    bool loadMap(string filepath);
    bool unloadMap();

    double get_angle(int tile);

    int solid_collision(Direction direction, SDL_Rect rect);
    int solid_collision(Direction direction, int x, int y, int w=TILESIZE, int h=TILESIZE);
    bool tryTeleporters(SDL_Rect* box, bool &reverseX, bool &reverseY);

    bool enemy_collision(SDL_Rect* hitbox);
    bool undogable_enemy_collision(SDL_Rect* hitbox);

    pair<int, string> item_collision(SDL_Rect* hitbox);

    bool tile_test(int type, int tile_x, int tile_y, SDL_Rect rect);
    bool tile_test(int type, int tile_x, int tile_y, int x, int y, int w, int h);

    bool is_done(int x, int y);
    void empty();
    void cleanup_stuff();
    void finish_map();

    int updateTotalPossibleScore();
};
#endif // MAP_H_INCLUDED
