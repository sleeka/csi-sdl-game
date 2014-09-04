#include "Map.h"
using namespace std;

Map::Map()
{
	height = 0;
	width = 0;
	mapFileLocation = "";
	tileSheet = new TileSheet();
	theme="underground";
	finished=false;
	startLoc.first = 96;
	startLoc.second = 96;
	player_type="";
	next_map="";
	gravity_delay = 1;
}

bool Map::unloadMap()
{
  cout << "\n> UNLOADING MAP\n";

  for(vector<Tile*>::iterator it = tiles.begin(); it!=tiles.end(); it++)
  {
    Tile* ptr = *it;
    ptr->~Tile();
    delete ptr;
  }
  tiles.clear();

  for(vector<Enemy*>::iterator it = enemies.begin(); it!=enemies.end(); it++)
  {
    Enemy* ptr = *it;
    delete ptr;
  }
  enemies.clear();
  // all undogable enemies are also contained in enemies
  undogableEnemies.clear();

  for(vector<Catcher*>::iterator it = catchers.begin(); it!=catchers.end(); it++)
  {
    Catcher* ptr = *it;
    delete ptr;
  }
  catchers.clear();

  for(vector<Item*>::iterator it = items.begin(); it!=items.end(); it++)
  {
    Item* ptr = *it;
    delete ptr;
  }
  items.clear();

  for(vector<Teleporter*>::iterator it = teleporters.begin(); it!=teleporters.end(); it++)
  {
    Teleporter* ptr = *it;
    delete ptr;
  }
  teleporters.clear();

  for(map< string, vector<Object*> >::iterator it=objectLayers.begin(); it!=objectLayers.end(); it++)
  {
    vector<Object*> vec = it->second;
    for(vector<Object*>::iterator it=vec.begin(); it!=vec.end(); it++)
    {
      Object* ptr = *it;
      delete ptr;
    }
  }
  objectLayers.clear();

  gravity_delay = 1;
  par_time = 20;

  // TileSheet frees memory before it assigns an image, so no need to do anything here
}

bool Map::loadMap(string filepath)
{
	cout << "\n> LOADING MAP: " << filepath;
	filepath = "data/maps/" + filepath;

	ifstream mapFile(filepath.c_str(), ifstream::in);

	if(mapFile == NULL)
	{
	  cout << " failed to open!\n";
    return false;
	}
	else
	{
		cout << " successfully opened...\n  > ";
		mapFileLocation = filepath;
		tiles.clear(); //TODO: clear layers of tiles correctly.
		objectLayers.clear();

    string tag = "";
    string str = "";

    int my_type;
		vector<Object*> currentObjects;

    while(getline(mapFile, tag))
    {
      if (tag == "THEME")
      {
        mapFile >> theme;
        cout << "theme = [" << theme << "] , ";
      }
      else if (tag == "SIZE")
      {
        mapFile >> width; //in tiles
        mapFile >> height; //in tiles
        cout << "size = [" << width << " x " << height << "] ";
      }
      else if (tag == "START")
      {
        mapFile >> startLoc.first;
        mapFile >> startLoc.second;
        cout << "start loc = [" << startLoc.first << " x " << startLoc.second << "] ";
      }
      else if (tag == "PLAYER")
      {
        mapFile >> player_type;
        cout << "player = [" << player_type << "] ";
      }
      else if(tag == "GRAVITY")
      {
        mapFile >> gravity_delay;
        cout << "gravity = [" << gravity_delay << "] ";
      }
      else if(tag == "NEXTMAP")
      {
        mapFile >> next_map;
        cout << "next map = [" << next_map << "] ";
      }
      else if(tag == "PAR_TIME")
      {
        mapFile >> par_time;
        cout << "par time = [" << par_time << "] ";
      }
      else if (tag == "ENEMIES")
      {
        cout<< "\n> Loading enemies...\n";
        while(str!="/ENEMIES")
        {
          getline(mapFile, str);
          if(str != "" && str!="/ENEMIES" && str[0] != '#')
          {
            Enemy* newEnem = new Enemy(str);
            string key;
            string value;
            do
            {
              getline(mapFile, str);
              key = str.substr(0, str.find_first_of("="));
              value = str.substr(str.find_first_of("=") + 1);
              if(key=="point")
              {
                pair<int,int> point;
                point.first = atoi(value.substr(0, value.find_first_of(",")).c_str());
                point.second = atoi(value.substr(value.find_first_of(",")+1).c_str());
                newEnem->addPoint(point);
              }
            }while(key=="point");

            if(newEnem->load_successful())
            {
              newEnem->startPath();
              enemies.push_back(newEnem);
              // for when player is on a catcher
              if(!newEnem->isDodgable())
              {
                undogableEnemies.push_back(newEnem);
              }
            }
            else
            {
              newEnem -> ~Enemy();
              delete newEnem;
            }
          }
        }
      }
      else if (tag == "OBJECT_LAYER")
      {
        string layer;
        mapFile >> layer;
        cout << "\n\n> Loading ojbect layer " << layer << "...\n";

        string type;
        string orientation;
        Direction orient;

        string end = "/" + layer;

        int x, y;
        mapFile >> type;
        while (type != end)
        {
        mapFile >> x;
        mapFile >> y;

        currentObjects.push_back(new Object(x, y, type));
        mapFile >> type;
        }

        objectLayers.insert(pair< string, vector<Object*> >(layer, currentObjects));
        currentObjects.clear();

      }
      else if (tag == "CATCHERS")
      {
        cout << "\n\n> Loading catchers...\n";
        string type;
        string end = "/" + tag;

        int x, y;
        mapFile >> type;
        while (type != end)
        {
        mapFile >> x;
        mapFile >> y;

        catchers.push_back(new Catcher(x, y, type));
        mapFile >> type;
        }
      }
      else if (tag == "ITEMS")
      {
        cout << "\n\n> Loading items...\n";
        string type;
        string end = "/" + tag;

        int x, y;
        mapFile >> type;
        while (type != end)
        {
        mapFile >> x;
        mapFile >> y;

        items.push_back(new Item(x, y, type));
        mapFile >> type;
        }
      }
      else if(tag == "TELEPORTERS")
      {
        string endtag = "/" + tag;
        string line, sA, sB, sEnd;
        Coord A, B, end;
        getline(mapFile, line);
        while(line!=endtag)
        {
          // cut up the line into three parts
          sA = line.substr(line.find_first_of("(")+1);
          sB = sA.substr(sA.find_first_of("(")+1);
          sEnd = sB.substr(sB.find_first_of("(")+1);
          // get the first coordinate of each
          A.x = atoi(sA.substr(0, sA.find_first_of(",")).c_str());
          B.x = atoi(sB.substr(0, sB.find_first_of(",")).c_str());
          end.x = atoi(sEnd.substr(0, sEnd.find_first_of(",")).c_str());
          // cut off the part we just read
          sA = sA.substr(sA.find_first_of(",")+1);
          sB = sB.substr(sB.find_first_of(",")+1);
          sEnd = sEnd.substr(sEnd.find_first_of(",")+1);
          // get the second coordinate of each
          A.y = atoi(sA.substr(0, sA.find_first_of(")")).c_str());
          B.y = atoi(sB.substr(0, sB.find_first_of(")")).c_str());
          end.y = atoi(sEnd.substr(0, sEnd.find_first_of(")")).c_str());

          teleporters.push_back(new Teleporter(A,B,end));
          getline(mapFile, line);
        }
      }
      else if(tag == "TERRAIN" )
      {
        cout << "\n> Loading tiles... ";
        for (int y=0; y<height; y++)
        {
          for(int x=0; x<width; x++)
          {
            mapFile >> my_type;
            if( mapFile.fail() || my_type<-10 || my_type >55 )
            {
              mapFile.close();
            }
            if(my_type == -1)
            {
              startLoc.first = x;
              startLoc.second = y;
            }
            tiles.push_back(new Tile(x,y, my_type)); // add the tile to the map;
          }
        }
        cout << "finished.";
      }
      else if(tag[0] == '#' || tag=="" || tag==" " || tag=="  ")
      {
          //skip this
      }
      else
      {
        cout << "\n> Unknown tag: " << tag << "\nMap loading ended early.\n\n";
        mapFile.close();
        return false;
      }
    }
		mapFile.close();
        cout << "\n> MAP LOADED SUCCESSFULLY\n";
		return true;
	}

	cout << "\n> DEFAULT MAP LOAD FAILED\n";
	mapFile.close();
	return false;
}

Map::~Map(){
}

void Map::loadObjectImages(Artist& artist)
{
  cout << "\n---------------LOADING IMAGES---------------\n";
  if(!objectLayers.empty())
  {
    map<string, vector<Object*> >::key_compare comp;

    string last=objectLayers.rbegin()->first;

    map<string, vector<Object*> >::iterator it = objectLayers.begin();
    do
    {
     cout << "\n> loading " << (*it).first << "\n";

      for(int i = 0; i < objectLayers[(*it).first].size(); i++)
      {
        objectLayers[(*it).first][i]->set_image(artist);
      }
    } while(comp((*it++).first, last));
  }
  if(!enemies.empty())
  {
    cout << "\n> loading enemies...\n";
    for(vector<Enemy*>::iterator it = enemies.begin(); it != enemies.end(); it++)
    {
      Enemy* ptr = *it;
      ptr->set_image(artist);
    }
  }
  if(!catchers.empty())
  {
    cout << "\n> loading catchers...\n";
    for(vector<Catcher*>::iterator it = catchers.begin(); it != catchers.end(); it++)
    {
      Object* ptr = *it;
      ptr->set_image(artist);
    }
  }
  if(!items.empty())
  {
    cout << "\n> loading items...\n";
    for(vector<Item*>::iterator item = items.begin(); item != items.end(); item++)
    {
      Item* ptr = *item;
      ptr -> set_image(artist);
    }
  }
}

void Map::update_enemies(int bullettime)
{
  if(enemies.empty())
    return;

  static int frame = 0;

  if(frame < 1)
  {
    frame++;
    return;
  }

  frame = 0;

  for(vector<Enemy*>::iterator it = enemies.begin(); it!=enemies.end(); it++)
  {
    Enemy* ptr = *it;
    ptr -> act(bullettime);
  }
}

pair<int, int> Map::get_start()
{
    return startLoc;
}

void Map::set_start(pair<int, int> start)
{
    startLoc.first = start.first;
    startLoc.second = start.second;

    if(startLoc.first < 0)
      startLoc.first = 0;
    if(startLoc.first > width*TILESIZE)
      startLoc.first = (width - 1)*TILESIZE;
    if(startLoc.second < 0)
      startLoc.second = 0;
    if(startLoc.second >= height*TILESIZE)
      startLoc.second = (height - 1)*TILESIZE;
}

vector<Tile*> Map::get_Tiles()
{
	return tiles;
}

Tile* Map::get_Tile(int x, int y)
{
	if(x < 0 || y < 0 || y >= height || x >= width)
			return NULL;
	return tiles[x+y*width];
}

Tile* Map::get_Tilepx(int x, int y)
{
	return get_Tile(x/TILESIZE, y/TILESIZE);
}

int Map::get_Width(){
	return width;
}

int Map::get_Height(){
	return height;
}

string Map::get_theme(){
  return theme;
}

int Map::get_gravity_delay()
{
  return gravity_delay;
}

void Map::set_theme(string th){
    theme=th;
}

int Map::get_Widthpx(){
	return width*TILESIZE;
}

int Map::get_Heightpx(){
	return height*TILESIZE;
}

void Map::set_Width(int newWidth)
{
  width = newWidth;
}

void Map::set_Height(int newHeight)
{
  height = newHeight;
}

int Map::get_score()
{
  return score;
}

int Map::get_par_time()
{
  return par_time;
}

void Map::set_score(int s)
{
  score = s;
}
string Map::get_player_type()
{
  return player_type;
}

vector<Item*>* Map::getItems()
{
  return &items;
}

vector<Catcher*>* Map::getCatchers()
{
  return &catchers;
}

void Map::cleanup_stuff()
{
  for(int i=0; i < items.size(); i++)
  {
    if(items.at(i)->is_dead())
    {
      items.erase(items.begin() + i);
      i--;
    }
  }
}

string Map::get_next_map()
{
  cout << "> Going to next map: " << next_map + ".map\n";
  return (next_map + ".map");
}

// returns the angle of a tile's slope in degrees, with
// a horizontal line with solid BELOW it as 0 degrees
double Map::get_angle(int tile)
{
  switch(tile)
  {
    // magic case numbers are the tile within the tileset sprite sheet
    // magic return numbers are the angle of aforementioned slope
    case 8:  case 33: return 0;
    case 3:  case 4:  return 22.5;
    case 19:          return 45;
    case 28: case 35: return 67.5;
    case 14: case 31: return 90;
    case 42: case 49: return 112.5;
    case 12:          return 135;
    case 24: case 25: return 157.5;
    case 22: case 30: return 180;
    case 26: case 27: return 202.5;
    case 11:          return 225;
    case 43: case 50: return 247.5;
    case 16: case 32: return 270;
    case 29: case 36: return 292.5;
    case 18:          return 315;
    case 5:  case 6:  return 337.5;
    default:          return 0;
  }
}
/// returns true if hitbox overlaps with anything that deals damage
bool Map::enemy_collision(SDL_Rect* hitbox)
{
  for(vector<Enemy*>::iterator enem = enemies.begin(); enem != enemies.end(); enem++)
  {
    Enemy* ptr = *enem;
    SDL_Rect* enem_box = ptr->get_bounds();
    if(enem_box->x <= (hitbox->x + hitbox->w) && (enem_box->x + enem_box->w) >= hitbox->x && (enem_box->y + enem_box->h) >= hitbox->y && enem_box->y <= (hitbox->y + hitbox->h))
      return true;
  }
  return false;
}
/// returns true if hitbox overlaps with anything that deals damage
bool Map::undogable_enemy_collision(SDL_Rect* hitbox)
{
  for(vector<Enemy*>::iterator enem = undogableEnemies.begin(); enem != undogableEnemies.end(); enem++)
  {
    Enemy* ptr = *enem;
    SDL_Rect* enem_box = ptr->get_bounds();
    if(enem_box->x <= (hitbox->x + hitbox->w) && (enem_box->x + enem_box->w) >= hitbox->x && (enem_box->y + enem_box->h) >= hitbox->y && enem_box->y <= (hitbox->y + hitbox->h))
      return true;
  }
  return false;
}
/// returns score and power if collided with item
pair<int, string> Map::item_collision(SDL_Rect* hitbox)
{
  for(vector<Item*>::iterator item = items.begin(); item != items.end(); item++)
  {
    Item* ptr = *item;
    SDL_Rect* item_box = ptr->get_bounds();
    if(item_box->x <= (hitbox->x + hitbox->w) && (item_box->x + item_box->w) >= hitbox->x && (item_box->y + item_box->h) >= hitbox->y && item_box->y <= (hitbox->y + hitbox->h))
    {
      pair<int, string> bundle;
      // returns point value and marks item for deletion
      bundle.first = ptr->pickup();
      // returns any powerup ability (as a string)
      bundle.second = ptr->get_power();
      return bundle;
    }
  }
  pair<int, string> nothing;
  return nothing;
}

int Map::solid_collision(Direction direction, SDL_Rect rect)
{
    solid_collision(direction, rect.x, rect.y, rect.w, rect.h);
}

/****************************************************************************
By the way, this tells us if a rectangle is colliding with a tile. We'll use
per-pixel iterative movement, so this tells us if a Rect is colliding  with
something if it moves one pixel in a Direction.
This all assumes that the rect passed in is not larger than a tile.
The integer returned represents the type of tile it is colliding with.
This is used for angle-bouncying mathy stuff.
****************************************************************************/
int Map::solid_collision(Direction direction, int x, int y, int w, int h){
  if(x<0)
    return 16;
  if(x+w > get_Widthpx())
    return 14;
  if(y<0)
    return 22;
  if(y+h > get_Heightpx())
    return 8;

  int type, tile_x, tile_y;
  tile_x = (x/TILESIZE)*TILESIZE;
  tile_y = (y/TILESIZE)*TILESIZE;

  //top left collision tile
  if(direction==UP || direction==LEFT || (x+3)%TILESIZE<6 || (y+3)%TILESIZE<6){
    type = get_Tiles()[x/TILESIZE + (y/TILESIZE*get_Width())]->get_type();
    //cout << "testing top left collision with     (" << tile_x/TILESIZE << "," << tile_y/TILESIZE << ") px:(" << tile_x << "," << tile_y << ") type " << type << ": ";
    if(type == 1 || type == 2 || (type>=7 && type<=9) || (type>=14 && type <= 16) || (type>=21 && type <= 23) || type == 34 || type == 41 || type == 10 || type == 13 || type == 17 || type == 20)
      switch(direction)
      {
        case UP: type = 22;break;
        case DOWN: type = 8;break;
        case LEFT: type = 16;break;
        case RIGHT: type = 14;break;
      }
    if(tile_test(type, tile_x, tile_y, x, y, w, h))
      return type;
  }
  //top right collision tile
  if(direction==UP || direction==RIGHT || (x+3)%TILESIZE<6 || (y+2)%TILESIZE<6){
    type = get_Tiles()[x/TILESIZE+ 1 + (y/TILESIZE*get_Width())]->get_type();
    //cout << "testing top right collision with    (" << tile_x/TILESIZE + 1 << "," << tile_y/TILESIZE << ") px:(" << tile_x+TILESIZE << "," << tile_y << ") type " << type << ": ";
      if(type == 1 || type == 2 || (type>=7 && type<=9) || (type>=14 && type <= 16) || (type>=21 && type <= 23) || type == 34 || type == 41 || type == 10 || type == 13 || type == 17 || type == 20)
      switch(direction)
      {
        case UP: type = 22;break;
        case DOWN: type = 8;break;
        case LEFT: type = 16;break;
        case RIGHT: type = 14;break;
      }
    if(tile_test(type, tile_x+TILESIZE, tile_y, x, y, w, h))
      return type;
  }
  //bottom left collision tile
  if(direction==DOWN||direction==LEFT || (x+3)%TILESIZE<6 || (y+3)%TILESIZE<6){
    type = get_Tiles()[x/TILESIZE + (((y)/TILESIZE+1)*get_Width())]->get_type();
    //cout << "testing bottom left collision with  (" << tile_x/TILESIZE << "," << tile_y/TILESIZE+1 << ") px:(" << tile_x << "," << tile_bottom << ") type " << type << ": ";
      if(type == 1 || type == 2 || (type>=7 && type<=9) || (type>=14 && type <= 16) || (type>=21 && type <= 23) || type == 34 || type == 41 || type == 10 || type == 13 || type == 17 || type == 20)
      switch(direction)
      {
        case UP: type = 22;break;
        case DOWN: type = 8;break;
        case LEFT: type = 16;break;
        case RIGHT: type = 14;break;
      }
    if(tile_test(type, tile_x, tile_y+TILESIZE, x, y, w, h))
      return type;
  }
  //bottom right collision tile
  if(direction==DOWN||direction==RIGHT || (x+3)%TILESIZE<6 || (y+3)%TILESIZE<6){
    type = get_Tiles()[x/TILESIZE + 1 + ((y/TILESIZE+1)*get_Width())]->get_type();
    //cout << "testing bottom right collision with (" << tile_x/TILESIZE+1 << "," << tile_y/TILESIZE+1 << ") px:(" << tile_x+TILESIZE << "," << tile_y+TILESIZE << ") type " << type << ": ";
      if(type == 1 || type == 2 || (type>=7 && type<=9) || (type>=14 && type <= 16) || (type>=21 && type <= 23) || type == 34 || type == 41 || type == 10 || type == 13 || type == 17 || type == 20)
      switch(direction)
      {
        case UP: type = 22;break;
        case DOWN: type = 8;break;
        case LEFT: type = 16;break;
        case RIGHT: type = 14;break;
      }
    if(tile_test(type, tile_x+TILESIZE, tile_y+TILESIZE, x, y, w, h))
      return type;
  }

  return false;
}

bool Map::tile_test(int type, int tile_x, int tile_y, SDL_Rect rect)
{
  return tile_test(type, tile_x, tile_y, rect.x, rect.y, rect.w, rect.h);
}

/******************************************************************************
This tells us if a Rect is overlapping with the solide area of a given tile
(by type). It's messy, but efficient. It should be called either two or four times
in one call to solid_collision(). The 4 times call occurs when the box is exactly
on the edge of one tile and another.
******************************************************************************/
bool Map::tile_test(int type, int tile_x, int tile_y, int x, int y, int w, int h){
  int r = x + w ;
  int b = y + h -1;
  int tile_right = tile_x+TILESIZE;
  int tile_bottom = tile_y+TILESIZE;

  if(!type)
      return false;
  else if(type==33){
    if((tile_bottom >= y)&&(tile_y+TILESIZE/2 <= b )&&(tile_right>=x)&&(tile_x<=r)){
        return true;
    }
  }else if(type==30){
    if((tile_y + TILESIZE/2 >= y)&&(tile_y <= b)&&(tile_right>=x)&&(tile_x<=r)){
        return true;
    }
  }else if(type==31){
    if((tile_bottom >= y)&&(tile_y <= b)&&(tile_right>=x)&&(tile_x+TILESIZE/2<=r)){
        return true;
    }
  }else if(type==32){
    if((tile_bottom >= y)&&(tile_y <= b)&&(tile_x+TILESIZE/2>=x)&&(tile_x<=r)){
        return true;
    }
  }else if (type==3){
    if(r >= tile_x && x <= tile_right && b >= tile_y+TILESIZE/2 && y<=tile_bottom){
      if(r <= tile_right && (b - tile_y - TILESIZE/2)*2 > (tile_right - r))
        return true;
      else if(r > tile_right)
        return true;
    }
  }else if (type==4){
    if(r >= tile_x && x <= tile_right && b >= tile_y && y<=tile_bottom){
      if(r <= tile_right && (b-tile_y)*2 > (tile_right - r))
        return true;
      else if(r > tile_right)
        return true;
    }
  }else if (type==5){
    if(r >= tile_x && x <= tile_right && b >= tile_y && y<=tile_bottom){
      if(x >= tile_x && (b-tile_y)*2 > (x - tile_x))
        return true;
      else if(x < tile_x)
        return true;
    }
  }else if (type==6){
    if(r >= tile_x && x <= tile_right && b >= tile_y+TILESIZE/2 && y<=tile_bottom){
      if(x >= tile_x && (b - tile_y - TILESIZE/2)*2 > (x - tile_x))
        return true;
      else if(x < tile_x)
        return true;
    }
  }else if (type==11){
    if(r >= tile_x && x <= tile_right && b >= tile_y && y<= tile_bottom){
      if(x >= tile_x &&(tile_right - x) >= (y - tile_y))
        return true;
      else if(x < tile_x)
        return true;
    }
  }else if (type==12){
    if(r >= tile_x && x <= tile_right && b >= tile_y && y<= tile_bottom){
      if(r <= tile_right && (r - tile_x) >= (y - tile_y))
        return true;
      else if(r > tile_right)
        return true;
    }
  }else if (type==18){
    if(r > tile_x && x < tile_right && b > tile_y && y<tile_bottom){
      if(x >= tile_x && (tile_right - x) >= (tile_bottom - b))
        return true;
      else if(x < tile_x)
        return true;
    }
  }else if (type==19){
    if(r > tile_x && x <= tile_right && b >= tile_y && y<=tile_bottom){
      if(r <= tile_right && (r - tile_x) >= (tile_bottom - b))
        return true;
      else if(r > tile_right)
        return true;
    }
  }else if (type==24){
    if(r >= tile_x && x <= tile_right && b >= tile_y && y<=tile_bottom-TILESIZE/2){
      if(r <= tile_right && (tile_bottom - y - TILESIZE/2)*2 > (tile_right - r))
        return true;
      else if(r > tile_right)
        return true;
    }
  }else if (type==25){
    if(r >= tile_x && x <= tile_right && b >= tile_y && y<=tile_bottom){
      if(r <= tile_right && (tile_bottom - y)*2 > (tile_right - r))
        return true;
      else if(r > tile_right)
        return true;
    }
  }else if (type==26){
    if(r >= tile_x && x <= tile_right && b >= tile_y && y<=tile_bottom){
      if(x >= tile_x && (tile_bottom - y)*2 > (x - tile_x))
        return true;
      else if(x < tile_x)
        return true;
    }
  }else if (type==27){
    if(r >= tile_x && x <= tile_right && b >= tile_y && y<= tile_bottom-TILESIZE/2){
      if(x >= tile_x && (tile_bottom - y - TILESIZE/2)*2 > (x - tile_x))
        return true;
      else if(x < tile_x)
        return true;
    }
  }else if (type==28){
    if(r >= tile_x+TILESIZE/2 && x <= tile_right && b >= tile_y && y<=tile_bottom){
      if(r <= tile_right && (r - tile_x - TILESIZE/2)*2 > (tile_bottom - b))
        return true;
      else if(r > tile_right)
        return true;
    }
  }else if (type==29){
    if(r >= tile_x && x <= tile_x+TILESIZE/2 && b >= tile_y && y<=tile_bottom){
      if(x >= tile_x && (tile_right - x - TILESIZE/2)*2 > (tile_bottom - b))
        return true;
      else if(x < tile_x)
        return true;
    }
  }else if (type==35){
    if(r >= tile_x && x <= tile_right && b >= tile_y && y<=tile_bottom){
      if(r <= tile_x+TILESIZE/2 && (r-tile_x)*2 > (tile_bottom-b))
        return true;
      else if(r > tile_x+TILESIZE/2)
        return true;
    }
  }else if (type==36){
    if(r >= tile_x && x <= tile_right && b >= tile_y && y<=tile_bottom){
      if(x >= tile_x+TILESIZE/2 && (tile_right - x)*2 > (tile_bottom-b))
        return true;
      else if(x < tile_x+TILESIZE/2)
        return true;
    }
  }else if (type==42){
    if(r >= tile_x && x <= tile_x && b >= tile_y && y<=tile_bottom){
      if(r <= tile_x+TILESIZE/2 && (r-tile_x)*2 > (y - tile_y))
        return true;
      else if(x > tile_x+TILESIZE/2)
        return true;
    }
  }else if (type==43){
    if(r >= tile_x && x <= tile_right && b >= tile_y && y<=tile_bottom){
      if(x >= tile_x +TILESIZE/2 && (tile_right - x)*2 > (y - tile_y))
        return true;
      else if(x < tile_x +TILESIZE/2)
        return true;
    }
  }else if (type==49){
    if(r >= tile_x+TILESIZE/2 && x <= tile_right && b >= tile_y && y<=tile_bottom){
      if(r >= tile_x +TILESIZE/2 && (r - tile_x - TILESIZE/2)*2 > (y - tile_y))
        return true;
      else if(r < tile_x +TILESIZE/2)
        return true;
    }
  }else if (type==50){
    if(r >= tile_x && x <= tile_x+TILESIZE/2 && b >= tile_y && y<= tile_bottom){
      if(x >= tile_x && (tile_right - x - TILESIZE/2)*2 > (y - tile_y))
        return true;
      else if(x < tile_x)
        return true;
    }
  }else if (type==52){
    finish_map();
    //cout<<"reached and exit.\n";
    return false;
  }else if((tile_bottom >= y)&&(tile_y<= b)&&(tile_right>=x)&&(tile_x<=r)){
    return true;
  }

  return false;
}

void Map::finish_map()
{

  finished=true;
}

/// Draw the distant backround
void Map::drawDistance(SDL_Rect* camera, SDL_Surface* screen)
{
  SDL_Rect tileLocation;
  tileLocation.h = 64;
  tileLocation.w = 64;

  for(int i=0; i < camera->w/64 + 1 ; i++)
  {
    for(int k=0; k < camera->h/64 + 2; k++)
    {
      tileLocation.x = i*64 - ((camera->x%(64*4))/4);
      tileLocation.y = k*64 - ((camera->y%(64*4))/4);
      SDL_BlitSurface( tileSheet->get_Surface(), &tileSheet->getDistanceTile(), screen, &tileLocation);
    }
  }
}

/// Draw the background using the bottom right corner of the tileSheet (2x2 squares across the entire background)
void Map::drawBackground(SDL_Rect* camera, SDL_Surface* screen)
{
  SDL_Rect tileLocation;
  tileLocation.h = 64;
  tileLocation.w = 64;

  for(int i=0; i < camera->w/64 + 1 ; i++)
  {
    for(int k=0; k < camera->h/64 + 2; k++)
    {
      tileLocation.x = i*64 - ((camera->x%(64*3))/3);
      tileLocation.y = k*64 - ((camera->y%(64*3))/3);
      SDL_BlitSurface( tileSheet->get_Surface(), &tileSheet->getBackgroundTile(), screen, &tileLocation);
    }
  }
}

/// Draws the objects of a specified layer (background/foreground) on a surface
void Map::drawObjects(SDL_Rect* camera, SDL_Surface* screen, string layer, bool lock_position)
{
  if(!objectLayers[layer].size())
  {
    //cout << "Layer \"" << layer << "\" is empty.\n";
    return;
  }

  for(int i = 0; i < objectLayers[layer].size(); i++)
  {
    objectLayers[layer][i]->display(*camera, screen, lock_position);
  }
}

/// Draws all of the maps tiles onto a surface
void Map::drawTiles(SDL_Rect* camera, SDL_Surface* screen)
{
  for(vector<Tile*>::iterator tile = tiles.begin(); tile != tiles.end(); tile++)
  {
    blitTile(screen, camera, *tile);
  }
}

/// Draws the map onto the screen by blitting visible tiles starting from the lowest layer first onto the screen surface.
void Map::drawMap(SDL_Surface* screen, SDL_Rect* camera)
{
  // Repeating background tiles
	drawDistance(camera, screen);
	// Distant objects
	drawObjects(camera, screen, "distance");
  // Repeating background tiles
	drawBackground(camera, screen);

  // Background objects
	drawObjects(camera, screen, "background");

  // Terrain tiles
  for(vector<Tile*>::iterator tile = tiles.begin(); tile != tiles.end(); tile++)
  {
    blitTile(screen, camera, *tile);
  }
  // The catchers
  for(vector<Catcher*>::iterator cat = catchers.begin(); cat != catchers.end(); cat++)
  {
    Object* ptr = *cat;
    ptr -> display(*camera, screen);
  }
  // The pickup items
  for(vector<Item*>::iterator item = items.begin(); item != items.end(); item++)
  {
    Item* ptr = *item;
    ptr -> display(*camera, screen);
  }
  // The enemies
  for(vector<Enemy*>::iterator enem = enemies.begin(); enem != enemies.end(); enem++)
  {
    Enemy* ptr = *enem;
    ptr -> display(*camera, screen);
  }
}


/// decide of the tile should be blitted and if so blit it to the screen.
void Map::blitTile(SDL_Surface* screen, SDL_Rect* camera, Tile* tile)
{
	if(	(tile->get_box()->x*TILESIZE >= camera->x-TILESIZE 				//if the tile is within the left side of the screen
		&& tile->get_box()->x*TILESIZE < camera->x+camera->w)				//if the tile is within the right side of the screen
		&& (tile->get_box()->y*TILESIZE >= camera->y-TILESIZE 			//if the tile is within the top of the screen
		&& tile->get_box()->y*TILESIZE < camera->y+camera->h))    //if the tile is within the bottom of the screen
    {
		//draw the tile to the screen
		SDL_BlitSurface( tileSheet->get_Surface(), &tileSheet->getTileFromSheet(tile), screen, &getTileScreenCoord(tile,camera));
	}
}


/// Get the screen coordinates of a tile given the tile and the current camera.
SDL_Rect Map::getTileScreenCoord(Tile* tile, SDL_Rect* camera)
{
	SDL_Rect screenCoord;
	screenCoord.x = tile->get_box()->x*TILESIZE - camera->x;
	screenCoord.y = tile->get_box()->y*TILESIZE - camera->y;
	return screenCoord;
}

void Map::set_TileSheet(Artist* artist, string filename)
{
	tileSheet->loadSheet(artist, filename);
}

/// assumes the exit point of a map is a tile of type 52
bool Map::is_done(int x, int y)
{
  if(    get_Tiles()[x/TILESIZE + (y/TILESIZE*get_Width())]->get_type()==52
      || get_Tiles()[x/TILESIZE+ 1 + (y/TILESIZE*get_Width())]->get_type()==52
      || get_Tiles()[x/TILESIZE + (((y)/TILESIZE+1)*get_Width())]->get_type()==52
      || get_Tiles()[x/TILESIZE + 1 + ((y/TILESIZE+1)*get_Width())]->get_type()==52)
    return true;
  return false;
}

vector<Tile*>* Map::get_TilesPointer(){
  return &tiles;
}

/// used by the editor when creating a blank map
void Map::empty()
{
  tiles.clear();

  for (int y=0; y<height; y++)
  {
    for(int x=0; x<width; x++)
    {
      tiles.push_back(new Tile(x,y, 0));
    }
  }

  for(vector<Catcher*>::iterator it = catchers.begin(); it!=catchers.end(); it++)
  {
    Catcher* ptr = *it;
    delete ptr;
  }
  catchers.clear();

  for(vector<Enemy*>::iterator it = enemies.begin(); it!=enemies.end(); it++)
  {
    Enemy* ptr = *it;
    delete ptr;
  }
  enemies.clear();

  for(vector<Item*>::iterator it = items.begin(); it!=items.end(); it++)
  {
    Item* ptr = *it;
    delete ptr;
  }
  items.clear();

  for(map< string, vector<Object*> >::iterator it=objectLayers.begin(); it!=objectLayers.end(); it++)
  {
    vector<Object*> vec = it->second;
    for(vector<Object*>::iterator it=vec.begin(); it!=vec.end(); it++)
    {
      Object* ptr = *it;
      ptr->~Object();
      delete ptr;
    }
  }
  objectLayers.clear();

  cout << "Completed initializing the map";
}

/// used by the editor when selecting map decoration objects
vector<Object*>* Map::getObjects(string layer)
{
  if(objectLayers.find(layer) == objectLayers.end())
  {
    cout << "Layer \"" << layer << "\" does not exist.\n";
    return NULL;
  }

  vector<Object*>* objectlayer = &objectLayers[layer];
  return objectlayer;
}

/// used by the editor when selecting enemies
vector<Enemy*>* Map::getEnemies()
{
  if(!enemies.size())
  {
    //cout << "there are no enemies in map.\n";
    //return NULL;
  }

  return &enemies;
}

/// iterates through teleporters, attempting to teleport from each onew
bool Map::tryTeleporters(SDL_Rect* box, bool &reverseX, bool &reverseY)
{
  for(int i = 0; i<teleporters.size(); i++)
  {
    if(teleporters[i]->teleport(box))
    {
      reverseX = teleporters[i]->reverseXDirection();
      reverseY = teleporters[i]->reverseYDirection();
      return true;
    }
  }
  return false;
}

vector<Teleporter*>* Map::getTeleporters()
{
  return &teleporters;
}

int Map::updateTotalPossibleScore()
{
  int sum = 0;
  for (int i=0; i<items.size(); i++)
    sum += items[i]->pointValue();

  return sum;
}
