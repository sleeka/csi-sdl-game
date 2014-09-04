#ifndef EDITOR_H_INCLUDED
#define EDITOR_H_INCLUDED

#include <iomanip>
#include <fstream>

#include "Artist.h"
#include "StateMachine.h"
#include "Map.h"
#include "Button.h"
#include "Pencil.h"

#define NODERADIUS 5

using namespace std;

enum View{BACKGROUND, TERRAIN, INTERACTIVE, FOREGROUND};
enum Tool{LINE, BRUSH, RECTANGLE, CYCLE};
enum EditTool{SELECT, PLUS};
enum InterMode{ENEMIES, CATCHERS, ITEMS};

class Editor
{
  public:
    Editor(Artist* artistPointer, StateMachine* statePointer);
    ~Editor(){ delete pencil; }

    void run();

  protected:
    StateMachine* stateMachine;
    Artist* artist;
    Camera camera;
    Map map;
    Pencil* pencil;

    SDL_Surface* screen;
    SDL_Event event;

    // view switches
    vector<Button*> icons;
    // dropdown options
    vector<Button*> options;
    // terrain tools
    vector<Button*> tools;

    // filled by reading editor_index
    // in populateObjectBanks
    vector<string> themes;
    vector<string> objectFiles;
    vector<Object*> objectBank;
    vector<Enemy*> enemyBank;
    vector<Item*> itemBank;
    vector<Catcher*> catcherBank;

    // indexes for object banks
    int objectBank_select;
    int enemyBank_select;
    int catcherBank_select;
    int itemBank_select;

    // for rectangle tool
    pair<int,int> marquee_start;
    Image tile_plus;
    Image tile_minus;

    // backround toolbar image
    Image icon_bg;
    // tools for interactive view
    Button icon_plus;
    Button icon_select;
    Button icon_interactive_cycle;
    Button icon_toggle_paths;
    // switching layer views
    Button icon_properties;
    Button icon_background;
    Button icon_terrain;
    Button icon_interactive;
    Button icon_foreground;
    // terrain tools
    Button icon_cycle;
    Button icon_painbrush;
    Button icon_pencil;
    Button icon_rectangle;
    Button icon_themes;
    // dropdown options
    Button option_divider1;
    Button option_divider2;
    Button option_New;
    Button option_Save;
    Button option_SaveAs;
    Button option_Load;
    Button option_Playtest;
    Button option_Properties;

    // height of the dropdown menu when opened
    int dropdown_height;

    // indexes for map's object containers
    int dist_select;
    int bg_select;
    int enemy_select;
    int path_point_select;
    int catcher_select;
    int item_select;
    int fg_select;

    // enumerations
    View view;
    Tool tool;
    EditTool edit_tool;
    InterMode inter_mode;

    string mapname;

    // various editor toggles
    bool option_menu;
    bool fill_marquee;
    bool empty_marquee;
    bool show_paths;
    bool preview;

    // editor execution flow
    bool loadFile(string mapname = "custom");
    void update();
    void display();
    void input();

    // map control
    void createDefaultMap();
    void saveMap(string filepath = "data/maps/custom.map");
    void resizeMap(int newWidth, int newHeight);
    void simplifyTiles();
    int simplify(int type);
    void interpretTerrain();
    int interpret(  int tile,
                    int topleft,
                    int top,
                    int topright,
                    int left,
                    int right,
                    int bottomleft,
                    int bottom,
                    int bottomright
                 );

    void validateCamera();

    // Cycle tool
    void tileCycle(bool rightClick);

    // Brush tool
    void brushAndErase();

    // marquee/rectangle tool functions
    void marqueeTiles(int x, int y);
    void drawMarquee(bool plus);

    // selecting and moving things
    void cycleObjects(bool forward);
    void selectObject();
    void populateObjectBanks();
    void removeSelected();
    void gridBasedNudge(Direction dir);
    void nudgePath(Enemy* path, Direction dir, int amount);
    void nudgeSelectionBG(Direction dir, int amount);
    void nudgeCatcher(Direction dir, int amount);
    void nudgeItem(Direction dir, int amount);
    void nudgeSelectionFG(Direction dir, int amount);
    void nudgeSelection(SDL_Rect*, Direction, int);

    // Interactive layer, enemy mode
    bool selectPathPoint(int x, int y);
    void drawPath(int index, Uint32 color);

};

#endif // EDITOR_H_INCLUDED
