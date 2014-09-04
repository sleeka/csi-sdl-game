#include "Enemy.h"

using namespace std;

Enemy::Enemy()
{
    type = "missing";
    facing_left = false;
    loop = false;
    trigger.first = -1;
    trigger.second = -1;
    forward = true;
    dodgable = true;
    node = 0;

    initFromFile();
}

Enemy::Enemy(string type)
{
    this->type = type;
    loop = false;
    trigger.first = -1;
    trigger.second = -1;
    forward = true;
    dodgable = true;
    node = 0;
    successful_load = initFromFile();
    facing_left = false;
}

Enemy::Enemy(Enemy& clone)
{
    bounds = *clone.get_bounds();
    speed = clone.get_speed();
    dodgable = clone.isDodgable();
}

// loads the Enemy's information from a .enemy text file
bool Enemy::initFromFile()
{
  string filepath = "data/enemy/" + type + ".enemy";

  cout << "Loading enemy " << filepath << ", ";

  ifstream enemyFile(filepath.c_str(), ifstream::in);

  if(enemyFile != NULL)
  {
    cout << "ifstream successfull... ";

    string tag = "";
    string line = "";
    while(getline(enemyFile, line))
    {
      if(line == "" || line==" " || line[0]==';')
      {
        //skip it
      }
      else if(line[0] == '[')
      {
        tag = line.substr(1, line.find_first_of("]") - 1);
      }
      else
      {
        // use key=value pairs like an ini file
        string key = line.substr(0, line.find_first_of("="));
        string value = line.substr(line.find_first_of("=") + 1);
        if(tag=="stats")
        {
          if(key == "size")
          {
            bounds.x=0;
            bounds.y=0;
            bounds.w = atoi(value.c_str());
            bounds.h = bounds.w;
            cout << "size = [" << bounds.w << " x " << bounds.h << "] ";
          }
        }
        else if (tag == "move")
        {
          if(key == "speed")
          {
            speed = atoi(value.c_str());
            cout << "speed = [" << speed << "] ";
          }
          // this might be usefull if we want some enemies to start
          // moving when the player crosses some line
          else if(key == "trigger")
          {
            if(value == "NONE" || value == "FALSE" || value == "none" || value == "false")
            {
              trigger.first = -1;
              trigger.second = -1;
              cout << "trigger disabled ";
            }
            else
            {
              trigger.first = 1;
              trigger.second = 1;
              cout << "trigger enabled ";
            }
          }
          else if(key == "loop")
          {
            loop = atoi(value.c_str());
            cout << "loop = " << (loop?"[TRUE]":"[FALSE]")<< " ";
          }
        }
        else if(tag == "special")
        {
          if(key == "dodgable")
          {
            dodgable = atoi(value.c_str());
            cout << "dodgeable = [" << dodgable << "] ";
          }
        }
        else
          cout << "skipping unknown tag \"" << tag << "\"";
      }
    }
    cout << "\n";
    // successfully loaded the file
    return true;
  }
  // couldn't find the file
  cout << "invalide input file.\n";
  return false;
}

void Enemy::clone(Enemy& clone)
{
  *this = clone;
}

Enemy Enemy::operator=(Enemy& clone)
{
  bounds = *clone.get_bounds();
  speed = clone.get_speed();

  return *this;
}

Enemy::~Enemy()
{
  // free any memory belonging to Enemy
}

void Enemy::startPath()
{
  // don't want them all to start at 0,0 when we load the map
  teleport(path[0].first - (bounds.w/2), path[0].second- (bounds.h/2));
}

/********************************************************************
This is called regularly by Engine.
It makes the enemy move along its path.
********************************************************************/
void Enemy::act(int slowmotion)
{
  // if slomotion powerup is active, only act once every five ticks
  if(slowmotion && slowmotion % 5 != 0)
    return;

  // node is the current point Enemy is moving towards
  // if it has reached the end of the path, it needs to
  // turn around or teleport to the beginning (depending on "loop")
  if(node > path.size() -1)
  {
    if(!loop)
    {
      forward = false;
      node = path.size() - 2;
      if(node < 0)
        node = 0;
    }
    else
    {
      node = 1;
      bounds.x = path[0].first - bounds.w/2;
      bounds.y = path[0].second - bounds.h/2;
    }
  }
  // if it was going backward and reached the beginning
  else if(node<=0)
  {
    // turn around
    forward = true;
    node = 0;
  }

  // find the coordinates of the center of the Enemy
  // ( uses top-left by default )
  int center_x = bounds.x + bounds.w/2;
  int center_y = bounds.y + bounds.h/2;

  if(path[node].first > center_x)
    facing_left = false;
  else if (path[node].first < center_x)
    facing_left = true;

  //cout << "Enemy: current coords are " << center_x << ", " << center_y << "\n";

  // if Enemy was loaded without specifying any points to move to
  if(path.empty())
  {
    cout << "enemy cannot move because it has no path\n";
    // we could throw some kind of exception but that's too much work
    return;
  }

  // if it has reached the intended point, go to the next one
  if(center_x == path[node].first && center_y == path[node].second)
  {
    if(forward)
      node++;
    else
      node--;
  }

  // if it has more than one point, get moving!
  if(path.size() > 1)
  {
    // there is some slight rounding error that may occasionaly
    // cause the enemy to backtrack for one frame
    double angle = atan2 (center_y - path[node].second, center_x - path[node].first);

    double inc_x = -1*abs(speed)*cos(angle);
    double inc_y = -1*abs(speed)*sin(angle);

    if(slowmotion)
    {
      inc_x *= 0.7;
      inc_y *= 0.7;
    }

    //cout << "Enemy move: inc_x = " << inc_x << " , inc_y = " << inc_y << " ; ";

    // use the sprite facing the direction it is going
    int x_sign = 1;

    if(inc_x<0)
    {
      inc_x *= -1;
      x_sign = -1;
    }

    int y_sign = 1;

    if(inc_y<0)
    {
      y_sign = -1;
      inc_y *= -1;
    }

    // round
    int x_dist = floor(inc_x + 0.5);
    int y_dist = floor(inc_y + 0.5);

    //cout << "x_dist = " << x_dist*x_sign << " , y_dist = " << y_dist*y_sign << "\n";

    // don't want to overshoot the point in one step
    // iteratively move the distance, stop if already there
    for(int i = 0; i < x_dist && (bounds.x + bounds.w/2)!= path[node].first; i++)
    {
      bounds.x += x_sign;
    }
    for(int i = 0; i < y_dist && (bounds.y + bounds.h/2)!= path[node].second; i++)
    {
      bounds.y += y_sign;
    }
  }
  // if there is only one point, the Enemy should teleport and stay there
  else if(path.size() == 1)
  {
    bounds.x = path[0].first - bounds.w/2;
    bounds.y = path[0].second - bounds.h/2;
  }

  // keep the animation going
  img.incFrame();
}

void Enemy::display(SDL_Rect camera, SDL_Surface* screen)
{
  img.display(screen, facing_left, bounds.x - camera.x, bounds.y - camera.y);
}

/// adds a coordinate to the Enemy's path
void Enemy::addPoint(pair<int,int> point)
{
  path.push_back(point);
}

/// loads an image for the Enemy and divides into frames
void Enemy::set_image(Artist& artist, string filepath)
{
  if(filepath == "" && type!="")
    filepath = "enemy/" + type + ".png";

  img.setImage(artist, filepath, true, true);

  img.frameSplice(bounds.w, bounds.h);
}

int Enemy::get_speed()
{
  return speed;
}

SDL_Rect* Enemy::get_bounds()
{
  return &bounds;
}


void Enemy::teleport(SDL_Rect box)
{
  bounds.x = box.x;
  bounds.y = box.y;
}

void Enemy::teleport(int x, int y)
{
  bounds.x = x;
  bounds.y = y;
}

vector<pair<int, int> >* Enemy::getPath()
{
  return &path;
}
