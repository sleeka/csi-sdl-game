#include "Editor.h"

Editor::Editor(Artist* artistPointer, StateMachine* statePointer)
{
  cout << "\n_______________EDITOR INITIALIZATION_______________\n";

  stateMachine = statePointer;
  artist = artistPointer;
  screen = artist -> getScreen();
  pencil = new Pencil(&camera, &map);
  option_menu = false;
  fill_marquee = false;
  empty_marquee = false;
  show_paths = false;
  preview = false;

  // for the rectangle tool
  marquee_start.first=-1;
  marquee_start.second=-1;

  // camera partially obscured by the menu, start above the top of the map
  camera.set_y(0 - BUTTONSIZE);

  // for use with the rectangle tool
  tile_plus.initialize(artist, "editor/tile_plus.png");
  tile_minus.initialize(artist, "editor/tile_minus.png");

  // the "background" of the menu bar
  icon_bg.initialize(artist, "editor/icon_bg.png");

  // the icons for switching views
  icon_properties.initialize(artist, "editor/icon_properties.png", true);
  icon_background.initialize(artist, "editor/icon_background.png", true);
  icon_terrain.initialize(artist, "editor/icon_terrain.png"), true;
  icon_interactive.initialize(artist, "editor/icon_interactive.png", true);
  icon_foreground.initialize(artist, "editor/icon_foreground.png", true);

  icons.push_back(&icon_properties);
  icons.push_back(&icon_background);
  icons.push_back(&icon_terrain);
  icons.push_back(&icon_interactive);
  icons.push_back(&icon_foreground);

  // icons for switching terrain tools
  icon_cycle.initialize(artist, "editor/icon_Cycle.png", true);
  icon_pencil.initialize(artist, "editor/icon_Pencil.png", true);
  icon_painbrush.initialize(artist, "editor/icon_Paintbrush.png", true);
  icon_rectangle.initialize(artist, "editor/icon_Rectangle.png", true);
  icon_themes.initialize(artist, "editor/icon_themes.png", true);

  tools.push_back(&icon_themes);
  tools.push_back(&icon_pencil);
  tools.push_back(&icon_rectangle);
  tools.push_back(&icon_painbrush);
  tools.push_back(&icon_cycle);

  // edit options
  icon_plus.initialize(artist, "editor/icon_plus.png", true);
  icon_select.initialize(artist, "editor/icon_Rectangle.png", true);
  icon_interactive_cycle.initialize(artist, "editor/icon_interactive_cycle.png", true);
  icon_toggle_paths.initialize(artist, "editor/icon_paths.png", true);

  // dropdown menu buttons
  option_divider1.initialize(artist, "editor/option_divider.png", false);
  option_divider2.initialize(artist, "editor/option_divider.png", false);
  option_New.initialize(artist, "editor/option_New.png", false);
  option_Save.initialize(artist, "editor/option_Save.png", false);
  option_SaveAs.initialize(artist, "editor/option_SaveAs.png", false);
  option_Load.initialize(artist, "editor/option_Load.png", false);
  option_Playtest.initialize(artist, "editor/option_Playtest.png", false);
  option_Properties.initialize(artist, "editor/option_Properties.png", false);

  options.push_back(&option_New);
  options.push_back(&option_divider1);
  options.push_back(&option_Save);
  options.push_back(&option_SaveAs);
  options.push_back(&option_Load);
  options.push_back(&option_divider2);
  options.push_back(&option_Playtest);
  options.push_back(&option_Properties);

  // find out the total height of the dropdown menu
  dropdown_height = 0;
  for(int i = 0; i < options.size(); i++)
  {
    options[i]->set_yOffset( BUTTONSIZE + dropdown_height );
    dropdown_height += options[i]->get_bounds().h/2;
  }

  // start out on terrain view
  view = TERRAIN;
  icons[2]->click();

  // start out with cycle tool
  tool = CYCLE;
  tools[3]->click();

  edit_tool = PLUS;
  icon_plus.click();

  dist_select = -1;
  bg_select = -1;
  fg_select = -1;
  enemy_select = -1;
  path_point_select = -1;
  catcher_select = -1;

  // prep interactive view for dealing with enemies
  inter_mode = ENEMIES;
  icon_interactive_cycle.force_up();

  objectBank_select = 0;
  enemyBank_select = 0;
  catcherBank_select = 0;
  itemBank_select = 0;

}

void Editor::createDefaultMap()
{
    map.set_theme("underground");
    map.set_Width(30);
    map.set_Height(30);
    map.empty();

    saveMap("data/maps/custom.map");
}

void Editor::saveMap(string filepath)
{
  cout << "> Saving map to " << filepath << "... ";
  ofstream save(filepath.c_str());

  if(!save)
  {
    cout << "cannot save map!\n";
    return;
  }

  cout << "successfully opened.\n";

  vector<Object*>* dist_objs = map.getObjects("distance");
  vector<Object*>* bg_objs = map.getObjects("background");
  vector<Object*>* fg_objs = map.getObjects("foreground");
  vector<Teleporter*>* teleporters = map.getTeleporters();

  save << "THEME\n" << map.get_theme() << "\n\n";
  save << "SIZE\n" << map.get_Width() << " " << map.get_Height() << "\n\n";
  save << "START\n" << map.get_start().first << " " << map.get_start().second << "\n\n";
  if(map.get_par_time())
    save << "PAR_TIME\n" << map.get_par_time() << "\n\n";
  if(map.get_theme() == "underwater")
    save << "PLAYER\n" << "pufferfish\n\n";
  else if(map.get_theme() == "ice")
    save << "PLAYER\n" << "penguin\n\n";
  else if(map.get_theme() == "underground")
    save << "PLAYER\n" << "turtle\n\n";
  else if(map.get_theme() == "space")
    save << "PLAYER\n" << "monkey\n\n";
  else
    save << "PLAYER\n" << map.get_player_type() << "\n\n";
  if(map.get_gravity_delay() != 1)
    save << "GRAVITY\n" << map.get_gravity_delay() << "\n\n";
  // format the nextmap string
  string nextmap = map.get_next_map();
  nextmap = nextmap.substr(0, nextmap.find_first_of("."));
  if(nextmap!= "")
    save << "NEXTMAP\n" << nextmap << "\n\n";

  // hold the tile number of exit tiles
  vector<int> exits;

  // use exit decorations to approximte exit tiles
  for(int i=0; i<bg_objs->size(); i++)
  {
    if(bg_objs->at(i)->get_type() == "DEC_Exit")
    {
      SDL_Rect* bnd = bg_objs->at(i)->get_bounds();
      int tile_x = (bnd->x + bnd->w/2)/TILESIZE;
      int tile_y = (bnd->y + bnd->h/2)/TILESIZE;
      exits.push_back(tile_x + tile_y*map.get_Width());
      cout << "exit detected at " << tile_x << ", " << tile_y << " ; will replace tile " << tile_x + tile_y*map.get_Width() << "\n";
    }
  }
  for(int i=0; i<fg_objs->size(); i++)
  {
    if(fg_objs->at(i)->get_type() == "DEC_Exit")
    {
      SDL_Rect* bnd = fg_objs->at(i)->get_bounds();
      int tile_x = (bnd->x + bnd->w/2)/TILESIZE;
      int tile_y = (bnd->y + bnd->h/2)/TILESIZE;
      exits.push_back(tile_x + tile_y*map.get_Width());
      cout << "exit detected at (" << tile_x << "," << tile_y << ") ; will replace tile " << tile_x + tile_y*map.get_Width() << "\n";
    }
  }

  vector<Tile*> tiles = map.get_Tiles();

  save << "TERRAIN\n";

  interpretTerrain();

  int tilenum = 0;

  // cycle through all the tiles
  for(int i = 0; i<tiles.size(); i++)
  {
    // go down when we get to the end of a row
    if(tilenum == map.get_Width())
    {
      tilenum = 1;
      save << "\n";
    }
    else
      tilenum++;

    int type = tiles[i]->get_type();
    // type 52 is the exit type
    for(int k = 0; k<exits.size() && type != 52; k++)
    {
      if(exits[k] == i)
      {
        // exit tile type
        type = 52;
        // don't need that exit anymore
        exits.erase(exits.begin() + k);
      }
    }
    // print the type of tile
    save << setfill ('0') << setw (2) << type << " ";
  }

  save << "\n";

  save << "\nOBJECT_LAYER\ndistance\n";
  if(dist_objs && !dist_objs->empty())
  {
    for(int i = 0; i < dist_objs->size(); i++)
    {
      SDL_Rect bounds = *dist_objs->at(i)->get_bounds();
      save << dist_objs->at(i)->get_type() << " " << bounds.x << " " << bounds.y << "\n";
    }
  }
  save << "/distance\n";

  save << "\nOBJECT_LAYER\nbackground\n";
  if(bg_objs && !bg_objs->empty())
  {
    for(int i = 0; i < bg_objs->size(); i++)
    {
      SDL_Rect bounds = *bg_objs->at(i)->get_bounds();
      save << bg_objs->at(i)->get_type() << " " << bounds.x << " " << bounds.y << "\n";
    }
  }
  save << "/background\n";

  save << "\nOBJECT_LAYER\nforeground\n";
  if(fg_objs && !fg_objs->empty())
  {
    for(int i = 0; i < fg_objs->size(); i++)
    {
      SDL_Rect bounds = *fg_objs->at(i)->get_bounds();
      save << fg_objs->at(i)->get_type() << " " << bounds.x << " " << bounds.y << "\n";
    }
  }
  save << "/foreground\n";

  vector<Enemy*>* enemies = map.getEnemies();

  if(enemies && !enemies->empty())
  {
    save << "\nENEMIES\n";
    for(int i = 0; i < enemies->size(); i++)
    {
      save << enemies->at(i)->get_type() << "\n";
      vector<pair<int, int> >* path = enemies->at(i)->getPath();
      for( int i = 0; i < path->size(); i++)
        save << "point=" << path->at(i).first << "," << path->at(i).second << "\n";
      save << "\n";
    }
    save << "/ENEMIES\n";
  }

  vector<Catcher*>* catchers = map.getCatchers();

  if(catchers && !catchers->empty())
  {
    save << "\nCATCHERS\n";
    for(int i = 0; i < catchers->size(); i++)
    {
      save << catchers->at(i)->get_type() << " ";
      SDL_Rect* bounds = catchers->at(i)->get_bounds();
      save << bounds->x << " " << bounds->y;
      save << "\n";
    }
    save << "/CATCHERS\n";
  }

  vector<Item*>* items = map.getItems();

  if(items && !items->empty())
  {
    save << "\nITEMS\n";
    for(int i = 0; i < items->size(); i++)
    {
      save << items->at(i)->get_type() << " ";
      SDL_Rect* bounds = items->at(i)->get_bounds();
      save << bounds->x << " " << bounds->y;
      save << "\n";
    }
      save << "/ITEMS\n";
  }

  if(teleporters && !teleporters->empty())
  {
    save << "\nTELEPORTERS\n";
    for(int i = 0; i < teleporters->size(); i++)
    {
      save << "(" << teleporters->at(i)->getA().x << "," << teleporters->at(i)->getA().y << ") ";
      save << "(" << teleporters->at(i)->getB().x << "," << teleporters->at(i)->getB().y << ") ";
      save << "(" << teleporters->at(i)->getEnd().x << "," << teleporters->at(i)->getEnd().y << ")\n";
    }
    save << "/TELEPORTERS\n";
  }
	save.close();
	cout << "Map saved without any problems.\n";
}

bool Editor::loadFile(string mapname)
{
  // free memory for any tiles and stuff
  map.unloadMap();

  // read in the new stuff from a file
  if(!map.loadMap(mapname + ".map"))
  {
    if (mapname=="custom" || !map.loadMap("custom.map"))
    {
      cout << "data/maps/custom.map is missing.\n******************\n";
      createDefaultMap();
      if (!map.loadMap("custom.map"))
      {
        cout << "Cannot create new custom.map!\n******************\n";
        return false;
      }
      cout << "New custom.map created and successfully loaded.\n******************\n";
    }
  }
  std::string filepath = "sprites/tileset/" + map.get_theme() + ".png";

  // get the image for the terrain
  map.set_TileSheet(artist, filepath);

  // get images for objects
  map.loadObjectImages(*artist);

  cout << "File finished loading into Editor.\n";
  return true;
}

void Editor::update()
{
  if(preview)
    map.update_enemies();
}

void Editor::display()
{
  // make sure the camera isn't out of bounds
  validateCamera();

  if(!option_menu)
  {
    /// draw repeating distant background
    map.drawDistance(camera.get_bounds(), screen);
    /// draw distant objects
    map.drawObjects(camera.get_bounds(), screen, "distance");
    /// draw repeating background
    map.drawBackground(camera.get_bounds(), screen);
    /// draw background objects
    map.drawObjects(camera.get_bounds(), screen, "background");
    /// draw terrain
    map.drawTiles(camera.get_bounds(), screen);
    /// display the catchers
    vector<Catcher*>* catchers = map.getCatchers();
    for(vector<Catcher*>::iterator cat = catchers->begin(); cat != catchers->end(); cat++)
    {
      Object* ptr = *cat;
      ptr -> display(*camera.get_bounds(), screen);
    }
    /// display the enemies
    vector<Enemy*>* enemies = map.getEnemies();
    for(vector<Enemy*>::iterator enem = enemies->begin(); enem != enemies->end(); enem++)
    {
      Enemy* ptr = *enem;
      ptr -> display(*camera.get_bounds(), screen);
    }
    /// display the items
    vector<Item*>* items = map.getItems();
    for(vector<Item*>::iterator it = items->begin(); it != items->end(); it++)
    {
      Item* ptr = *it;
      ptr -> display(*camera.get_bounds(), screen);
    }
    /// draw foreground objects
    map.drawObjects(camera.get_bounds(), screen, "foreground");

    int mx, my;
    SDL_GetMouseState(&mx, &my);

    if(view == BACKGROUND)
    {
      /// display background object selection box
      if(edit_tool == SELECT && bg_select >= 0 && bg_select < map.getObjects("background")->size())
      {
        SDL_Rect box = *map.getObjects("background")->at(bg_select)->get_bounds();
        int speed = map.getObjects("background")->at(bg_select)->get_scrollspeed();
        if(speed > 1)
        {
          box.x += camera.get_x()*(speed-1)/speed;
          box.y += camera.get_y()*(speed-1)/speed;
        }
        rectangleColor(screen, box.x - camera.get_x(), box.y - camera.get_y(), box.x + box.w - camera.get_x(), box.y + box.h - camera.get_y(), (Uint32)0x00FF00FF);
      }
      else if(edit_tool == SELECT && dist_select >= 0 && dist_select < map.getObjects("distance")->size())
      {
        SDL_Rect box = *map.getObjects("distance")->at(dist_select)->get_bounds();
        box.x += camera.get_x()*3/4;
        box.y += camera.get_y()*3/4;
        rectangleColor(screen, box.x - camera.get_x(), box.y - camera.get_y(), box.x + box.w - camera.get_x(), box.y + box.h - camera.get_y(), (Uint32)0x00FF00FF);
      }
      /// display background object preview on mouse
      if(edit_tool == PLUS && my > BUTTONSIZE && objectBank_select >= 0 && objectBank_select < objectBank.size())
      {
        SDL_Rect* objbox = objectBank[objectBank_select]->get_bounds();
        objbox->x = camera.get_bounds()->x + mx - (objbox->w/2);
        objbox->y = camera.get_bounds()->y + my - (objbox->h/2);
        objectBank[objectBank_select]->display(*camera.get_bounds(), screen, true);
      }
    }
    /// display all paths if show_paths is on
    if(show_paths)
    {
      // draw each path unless it is selected
      for(int i = 0; i < map.getEnemies()->size(); i++)
      {
        if(i != enemy_select)
          drawPath(i, (Uint32)0xA6F1C4FF);
      }
    }
    if(view == TERRAIN)
    {
      /// draw tool stuff
      if(tool == LINE && pencil->isDrawing() && !option_menu)
      {
        pencil->drawOutline(screen);
        if(pencil->readyToInterpret)
        {
          pencil->readyToInterpret=false;
          interpretTerrain();
        }
      }
      else if(fill_marquee)
        drawMarquee(true);
      else if(empty_marquee)
        drawMarquee(false);
    }
    else if(view == INTERACTIVE)
    {
      /// display selection box on active catcher
      if(edit_tool == SELECT && inter_mode == CATCHERS && catcher_select >= 0 && catcher_select < map.getCatchers()->size())
      {
        SDL_Rect box = *map.getCatchers()->at(catcher_select)->get_bounds();
        rectangleColor(screen, box.x - camera.get_x(), box.y - camera.get_y(), box.x + box.w - camera.get_x(), box.y + box.h - camera.get_y(), (Uint32)0x00FF00FF);
      }
      /// display selection box and path of selected enemy
      if(edit_tool == SELECT && inter_mode == ENEMIES && enemy_select >= 0 && enemy_select < map.getEnemies()->size())
      {
        // draw bounding box on selected enemy
        SDL_Rect box = *map.getEnemies()->at(enemy_select)->get_bounds();
        rectangleColor(screen, box.x - camera.get_x(), box.y - camera.get_y(), box.x + box.w - camera.get_x(), box.y + box.h - camera.get_y(), (Uint32)0x00FF00FF);
        // draw movement path for selected enemy
        drawPath(enemy_select, (Uint32)0x00FF96FF);
      }
      /// display selection box on selected item
      if(edit_tool == SELECT && inter_mode == ITEMS && item_select >= 0 && item_select < map.getItems()->size())
      {
        SDL_Rect box = *map.getItems()->at(item_select)->get_bounds();
        rectangleColor(screen, box.x - camera.get_x(), box.y - camera.get_y(), box.x + box.w - camera.get_x(), box.y + box.h - camera.get_y(), (Uint32)0x00FF00FF);
      }
      /// display addable catcher preview on mouse
      if(edit_tool == PLUS && my > BUTTONSIZE && inter_mode == CATCHERS && catcherBank_select >= 0 && catcherBank_select < catcherBank.size())
      {
        SDL_Rect* objbox = catcherBank[catcherBank_select]->get_bounds();
        objbox->x = camera.get_bounds()->x + mx - (objbox->w/2);
        objbox->y = camera.get_bounds()->y + my - (objbox->h/2);
        catcherBank[catcherBank_select]->display(*camera.get_bounds(), screen, true);
      }
      /// display addable enemy preview on mouse
      else if(edit_tool == PLUS && my > BUTTONSIZE && inter_mode == ENEMIES && enemyBank_select >= 0 && enemyBank_select < enemyBank.size())
      {
        SDL_Rect* objbox = enemyBank[enemyBank_select]->get_bounds();
        objbox->x = camera.get_bounds()->x + mx - (objbox->w/2);
        objbox->y = camera.get_bounds()->y + my - (objbox->h/2);
        enemyBank[enemyBank_select]->display(*camera.get_bounds(), screen);
      }
      /// display addable item preview on mouse
      else if(edit_tool == PLUS && my > BUTTONSIZE && inter_mode == ITEMS && itemBank_select >= 0 && itemBank_select < itemBank.size())
      {
        SDL_Rect* objbox = itemBank[itemBank_select]->get_bounds();
        objbox->x = camera.get_bounds()->x + mx - (objbox->w/2);
        objbox->y = camera.get_bounds()->y + my - (objbox->h/2);
        itemBank[itemBank_select]->display(*camera.get_bounds(), screen);
      }
    }
    else if(view == FOREGROUND)
    {
      /// display foreground selection box
      if(fg_select >= 0 && fg_select < map.getObjects("foreground")->size())
      {
        SDL_Rect box = *map.getObjects("foreground")->at(fg_select)->get_bounds();
        int speed = map.getObjects("foreground")->at(fg_select)->get_scrollspeed();
        if(speed > 1)
        {
          box.x += camera.get_x()*(speed-1)/speed;
          box.y += camera.get_y()*(speed-1)/speed;
        }
        rectangleColor(screen, box.x - camera.get_x(), box.y - camera.get_y(), box.x + box.w - camera.get_x(), box.y + box.h - camera.get_y(), (Uint32)0x00FF00FF);
      }
      /// display object preview
      if(edit_tool == PLUS && my > BUTTONSIZE && objectBank_select >= 0 && objectBank_select < objectBank.size())
      {
        SDL_Rect* objbox = objectBank[objectBank_select]->get_bounds();
        objbox->x = camera.get_bounds()->x + mx - (objbox->w/2);
        objbox->y = camera.get_bounds()->y + my - (objbox->h/2);
        objectBank[objectBank_select]->display(*camera.get_bounds(), screen, true);
      }
    }
  }
  else if(!options.empty())
  {
    /// dropdown menu buttons
    for(int i = 0; i < options.size(); i++)
    {
      options[i]->display(screen);
    }
  }
  /// draw menu bar buttons
  int t=0;
  for(int i = 0; (i+1)*BUTTONSIZE <= WINDOW_WIDTH; i++)
  {
    if(i<icons.size())
    {
      icons[i]->display(screen, i*BUTTONSIZE);
    }
    else if((view!=TERRAIN && (i+3)*BUTTONSIZE <= WINDOW_WIDTH)|| (i+1 + tools.size())*BUTTONSIZE <= WINDOW_WIDTH)
      icon_bg.display(screen,false,i*BUTTONSIZE);
    else if(view==TERRAIN && (i+1+tools.size()-t)*BUTTONSIZE > WINDOW_WIDTH)
    {
      tools[t]->display(screen, i*BUTTONSIZE);
      t++;
    }
  }
  /// draw other tool buttons
  if(view!=TERRAIN)
  {
    icon_plus.display(screen, WINDOW_WIDTH-BUTTONSIZE);
    icon_select.display(screen, WINDOW_WIDTH-BUTTONSIZE*2);
    icon_toggle_paths.display(screen, WINDOW_WIDTH-BUTTONSIZE*3);
    if(view == INTERACTIVE)
    {
      icon_interactive_cycle.display(screen, WINDOW_WIDTH-BUTTONSIZE*4);
    }
  }

}

/// iterates the bank of objects available to be placed
void Editor::cycleObjects(bool forward)
{
  if(edit_tool != PLUS)
      return;
  if(view == BACKGROUND || view == FOREGROUND)
  {
    if(!forward)
      objectBank_select--;
    else
      objectBank_select++;

    if(objectBank_select < 0)
      objectBank_select = objectBank.size()-1;
    else if(objectBank_select >= objectBank.size())
      objectBank_select = 0;
  }
  else if(view == INTERACTIVE)
  {
    // catcher select mode
    if(inter_mode == CATCHERS)
    {
      if(!forward)
        catcherBank_select--;
      else
        catcherBank_select++;

      if(catcherBank_select < 0)
        catcherBank_select = catcherBank.size()-1;
      else if(catcherBank_select >= catcherBank.size())
        catcherBank_select = 0;
    }
    // enemy select mode
    else if(inter_mode == ENEMIES)
    {
      if(!forward)
        enemyBank_select--;
      else
        enemyBank_select++;

      if(enemyBank_select < 0)
        enemyBank_select = enemyBank.size()-1;
      else if(enemyBank_select >= enemyBank.size())
        enemyBank_select = 0;
    }
    // item select mode
    else if(inter_mode == ITEMS)
    {
      if(!forward)
        itemBank_select--;
      else
        itemBank_select++;

      if(itemBank_select < 0)
        itemBank_select = itemBank.size()-1;
      else if(itemBank_select >= itemBank.size())
        itemBank_select = 0;
    }
  }
}

/// get user input and respond
void Editor::input()
{
  Uint8 *keystate = SDL_GetKeyState(NULL);
  int mx, my;
  bool mousedown = SDL_GetMouseState(&mx, &my)&SDL_BUTTON(1);
  while( SDL_PollEvent( &event ) )
  {
    // if the user clicks the X then just quit everything
    if( event.type == SDL_QUIT)
      stateMachine->setState(QUIT);
    else if( event.type == SDL_KEYDOWN )
    {
      switch(event.key.keysym.sym)
      {
        case SDLK_q: // both 'q' and 'ESC' pop us back to the main menu
        case SDLK_ESCAPE: stateMachine -> setState(MAIN_MENU); break;
        case SDLK_LESS: // continue
        case SDLK_COMMA: cycleObjects(false); break;
        case SDLK_GREATER: // continue
        case SDLK_PERIOD: cycleObjects(true); break;
        case SDLK_DELETE: removeSelected(); break;
        case SDLK_p: show_paths = !show_paths; break;
        case SDLK_SPACE: preview = true;
        case SDLK_LCTRL: // remove the last point of the pencil outline
          if(tool == LINE)
            pencil->removeLastCoord();
          break;
        /// arrow keys resize the map if CTRL is down, nudge things otherwise
        case SDLK_UP:
          if((keystate[SDLK_LCTRL] || keystate[SDLK_RCTRL]) && view == TERRAIN)
            resizeMap(map.get_Width(), map.get_Height() - 1);
          else if(keystate[SDLK_LSHIFT] || keystate[SDLK_RSHIFT])
            gridBasedNudge(UP);
          break;
        case SDLK_DOWN:
          if((keystate[SDLK_LCTRL] || keystate[SDLK_RCTRL]) && view == TERRAIN)
            resizeMap(map.get_Width(), map.get_Height() + 1);
          else if(keystate[SDLK_LSHIFT] || keystate[SDLK_RSHIFT])
            gridBasedNudge(DOWN);
          break;
        case SDLK_LEFT:
          if((keystate[SDLK_LCTRL] || keystate[SDLK_RCTRL]) && view == TERRAIN)
            resizeMap(map.get_Width() -1, map.get_Height());
          else if(keystate[SDLK_LSHIFT] || keystate[SDLK_RSHIFT])
            gridBasedNudge(LEFT);
          break;
        case SDLK_RIGHT:
          if((keystate[SDLK_LCTRL] || keystate[SDLK_RCTRL]) && view == TERRAIN)
            resizeMap(map.get_Width() +1, map.get_Height());
          else if(keystate[SDLK_LSHIFT] || keystate[SDLK_RSHIFT])
            gridBasedNudge(RIGHT);
          break;
        default: break;
      }
    }
    else if(event.type == SDL_KEYUP)
    {
      if(event.key.keysym.sym == SDLK_SPACE)
      {
        preview = false;
      }
    }
    else if(event.type == SDL_MOUSEBUTTONDOWN)
    {
        if(my<0 || my > WINDOW_HEIGHT || mx<0 || mx>WINDOW_WIDTH)
          return;

        switch(event.button.button)
        {
          case SDL_BUTTON_LEFT:
            /// clicking on menu buttons
            if(my < BUTTONSIZE)
            {
              // first icon, properties
              if(mx < BUTTONSIZE)
              {
                if(icons[0]->isDown())
                {
                  option_menu = false;
                  icons[0]->force_up();
                }
                else
                {
                  option_menu = true;
                  icons[0]->click();
                }
              }
              // second icon, background
              else if(mx < 2*BUTTONSIZE)
              {
                view = BACKGROUND;
                for(int i =1; i < icons.size(); i++)
                {
                  if(i == 1)
                    icons.at(i)->click();
                  else
                    icons.at(i)->force_up();
                }
              }
              // third icon, terrain
              else if(mx < 3*BUTTONSIZE)
              {
                view = TERRAIN;
                for(int i =1; i < icons.size(); i++)
                {
                  if(i == 2)
                    icons.at(i)->click();
                  else
                    icons.at(i)->force_up();
                }
              }
              // fourth icon, interactive
              else if(mx < 4*BUTTONSIZE)
              {
                view = INTERACTIVE;
                for(int i =1; i < icons.size(); i++)
                {
                  if(i == 3)
                    icons.at(i)->click();
                  else
                    icons.at(i)->force_up();
                }
              }
              // fifth icon, foreground
              else if(mx < 5*BUTTONSIZE)
              {
                view = FOREGROUND;
                for(int i =1; i < icons.size(); i++)
                {
                  if(i == 4)
                    icons.at(i)->click();
                  else
                    icons.at(i)->force_up();
                }
              }
              /// clicking on terrain tools
              else if(view == TERRAIN)
              {
                // first from the right, cycle
                if(mx>WINDOW_WIDTH-BUTTONSIZE)
                {
                  tool = CYCLE;
                  for(int i =0; i < tools.size(); i++)
                  {
                    if(i == 4)
                      tools.at(i)->click();
                    else
                      tools.at(i)->force_up();
                  }
                }
                // second from the right, brush
                else if(mx>WINDOW_WIDTH - 2*BUTTONSIZE)
                {
                  tool = BRUSH;
                  for(int i =0; i < tools.size(); i++)
                  {
                    if(i == 3)
                      tools.at(i)->click();
                    else
                      tools.at(i)->force_up();
                  }
                }
                // third from the right, rectangle
                else if(mx>WINDOW_WIDTH - 3*BUTTONSIZE)
                {
                  tool = RECTANGLE;
                  for(int i =0; i < tools.size(); i++)
                  {
                    if(i == 2)
                      tools.at(i)->click();
                    else
                      tools.at(i)->force_up();
                  }
                }
                // fourth from the right, pencil
                else if(mx>WINDOW_WIDTH - 4*BUTTONSIZE)
                {
                  tool = LINE;
                  for(int i =0; i < tools.size(); i++)
                  {
                    if(i == 1)
                      tools.at(i)->click();
                    else
                      tools.at(i)->force_up();
                  }
                }
                // fifth from the right, themes
                else if(mx>WINDOW_WIDTH - 5*BUTTONSIZE)
                {
                  // find which position in themes the current theme is
                  int current_theme = -1;
                  for(int i = 0;i<themes.size(); i++)
                  {
                    if(themes[i] == map.get_theme())
                      current_theme = i;
                  }
                  // attempt to load the next theme in the list
                  if(current_theme>-1)
                  {
                    current_theme++;
                    if(current_theme >= themes.size())
                      current_theme=0;
                    map.set_theme(themes[current_theme]);
                    map.set_TileSheet(artist, "sprites/tileset/" + map.get_theme() + ".png");
                    // reload the list of things we can insert (based on theme)
                    populateObjectBanks();
                  }
                }
              }
              /// clicking on general tools
              else
              {
                // first from the right, plus
                if(mx>WINDOW_WIDTH-BUTTONSIZE)
                {
                  edit_tool = PLUS;
                  icon_plus.click();
                  icon_select.force_up();
                }
                // second from the right, select
                else if(mx>WINDOW_WIDTH - 2*BUTTONSIZE)
                {
                  edit_tool = SELECT;
                  icon_select.click();
                  icon_plus.force_up();
                }
                // third from the right, toggle show all paths
                else if(mx>WINDOW_WIDTH - 3*BUTTONSIZE)
                {
                  show_paths = !show_paths;
                  if(show_paths)
                    icon_toggle_paths.click();
                  else
                    icon_toggle_paths.force_up();
                }
                // fourth from the right, interactive cycle
                else if(mx>WINDOW_WIDTH - 4*BUTTONSIZE && view == INTERACTIVE)
                {
                  if(view == INTERACTIVE)
                  {
                    if(inter_mode == ENEMIES)
                    {
                      // second frame
                      icon_interactive_cycle.mouseover();
                      inter_mode = CATCHERS;
                    }
                    else if(inter_mode == CATCHERS)
                    {
                      // third frame
                      icon_interactive_cycle.click();
                      inter_mode = ITEMS;
                    }
                    else if(inter_mode == ITEMS)
                    {
                      // first frame
                      icon_interactive_cycle.force_up();
                      inter_mode = ENEMIES;
                    }
                  }
                }
              }
            }
            else if(option_menu)
            {
              if(my < dropdown_height + BUTTONSIZE &&  mx < options[0]->get_bounds().w)
              {
                for(int i=0; i<options.size() && my > options[i]->get_bounds().y; i++)
                {
                  if(my < options[i]->get_bounds().y + options[i]->get_bounds().h/2)
                    switch(i)
                    {
                      // "New"
                      case 0: map.empty();
                              option_menu=false;
                              icons[0]->force_up();
                              break;
                      // "Save"
                      case 2: saveMap("data/maps/" + mapname + ".map");
                              option_menu=false;
                              icons[0]->force_up();
                              break;
                      // "Save As..." currently same as "Save"
                      case 3: saveMap("data/maps/" + mapname + ".map");
                              option_menu=false;
                              icons[0]->force_up();
                              break;
                      // the dividers don't do anything.
                      default: break;
                    }
                }
              }
              else
              {
                option_menu=false;
                icons[0]->force_up();
              }
            }
            else if(view == BACKGROUND)
            {
              if(edit_tool == SELECT)
                selectObject();
              else if(edit_tool == PLUS && objectBank_select >= 0 && objectBank_select<objectBank.size())
              {
                int stamp_x = mx + camera.get_bounds()->x - (objectBank[objectBank_select]->get_bounds()->w/2) ;
                int stamp_y = my + camera.get_bounds()->y - (objectBank[objectBank_select]->get_bounds()->h/2);
                Object* stamp = new Object(stamp_x, stamp_y, objectBank[objectBank_select]->get_type());
                if(stamp)
                {
                  string layer;
                  if(stamp->get_scrollspeed() == 4)
                  {
                    layer = "distance";
                    stamp->get_bounds()->x -= camera.get_x()*3/4;
                    stamp->get_bounds()->y -= camera.get_y()*3/4;
                  }
                  else
                    layer = "background";

                  cout << "Getting " << layer << " object vector to add " << stamp->get_type() << "\n";
                  vector<Object*>* obs = map.getObjects(layer);
                  if(obs)
                  {
                    cout << "adding " << stamp->get_type() << " to " << layer << " \n";
                    obs->push_back(stamp);
                    obs->at(obs->size() - 1)->set_image(*artist);
                  }
                }
              }
            }
            else if(view == TERRAIN)
            {
              switch(tool)
              {
                case CYCLE: simplifyTiles(); tileCycle(false); break;
                case RECTANGLE:
                  cout << "LClicking with rectangle tool...\n";
                  if( !fill_marquee && !empty_marquee)
                  {
                    cout << "Got in!\n";
                    fill_marquee = true;
                    marquee_start.first = mx + camera.get_bounds()->x;
                    marquee_start.second = my + camera.get_bounds()->y;
                  }break;
                case LINE: pencil->toggleDrawing(); break;
                default: break;
              }
              interpretTerrain();
            }
            else if(view == INTERACTIVE)
            {
              if(edit_tool == SELECT)
              {
                if(inter_mode != ENEMIES || !selectPathPoint(mx + camera.get_bounds()->x, my + camera.get_bounds()->y))
                  selectObject();
              }
              else if(edit_tool == PLUS && inter_mode == CATCHERS && catcherBank_select >= 0 && catcherBank_select<catcherBank.size())
              {
                int stamp_x = mx + camera.get_bounds()->x - (catcherBank[catcherBank_select]->get_bounds()->w/2);
                int stamp_y = my + camera.get_bounds()->y - (catcherBank[catcherBank_select]->get_bounds()->h/2);
                Catcher* stamp = new Catcher(stamp_x, stamp_y, catcherBank[catcherBank_select]->get_type());
                if(stamp)
                {
                  vector<Catcher*>* catchers = map.getCatchers();
                  if(catchers)
                  {
                    catchers->push_back(stamp);
                    catchers->at(catchers->size() - 1)->set_image(*artist);
                  }
                }
              }
              else if(edit_tool == PLUS && inter_mode == ENEMIES && enemyBank_select >= 0 && enemyBank_select<enemyBank.size())
              {
                int stamp_x = mx + camera.get_bounds()->x;
                int stamp_y = my + camera.get_bounds()->y;
                Enemy* stamp = new Enemy(enemyBank[enemyBank_select]->get_type());
                if(stamp && stamp->load_successful())
                {
                  pair<int, int> point;
                  point.first = stamp_x;
                  point.second = stamp_y;
                  stamp->addPoint(point);
                  stamp->startPath();
                  vector<Enemy*>* enemies = map.getEnemies();
                  if(enemies)
                  {
                    enemies->push_back(stamp);
                    enemies->at(enemies->size() - 1)->set_image(*artist);
                  }
                }
              }
              else if(edit_tool == PLUS && inter_mode == ITEMS && itemBank_select >= 0 && itemBank_select<itemBank.size())
              {
                int stamp_x = mx + camera.get_bounds()->x - (itemBank[itemBank_select]->get_bounds()->w/2) ;
                int stamp_y = my + camera.get_bounds()->y - (itemBank[itemBank_select]->get_bounds()->h/2);
                Item* stamp = new Item(stamp_x, stamp_y, itemBank[itemBank_select]->get_type());
                if(stamp)
                {
                  vector<Item*>* items = map.getItems();
                  if(items)
                  {
                    items->push_back(stamp);
                    items->at(items->size() - 1)->set_image(*artist);
                  }
                }
              }
            }
            else if(view == FOREGROUND)
            {
              if(edit_tool == SELECT)
                selectObject();
              else if(edit_tool == PLUS && objectBank_select >= 0 && objectBank_select<objectBank.size())
              {
                int stamp_x = mx + camera.get_bounds()->x - (objectBank[objectBank_select]->get_bounds()->w/2) ;
                int stamp_y = my + camera.get_bounds()->y - (objectBank[objectBank_select]->get_bounds()->h/2);
                Object* stamp = new Object(stamp_x, stamp_y, objectBank[objectBank_select]->get_type());
                if(stamp)
                {
                  //cout << "Getting background object vector to add " << stamp->get_type() << "\n";
                  vector<Object*>* bg = map.getObjects("foreground");
                  if(bg)
                  {
                    //cout << "adding " << stamp->get_type() << " to foreground\n";
                    bg->push_back(stamp);
                    bg->at(bg->size() - 1)->set_image(*artist);
                  }
                }
              }
            }
            break;

          case SDL_BUTTON_RIGHT:
            if(my > BUTTONSIZE)
            {
              if(view == TERRAIN)
              {
                switch(tool)
                {
                  case CYCLE: simplifyTiles(); tileCycle(true); break;
                  case RECTANGLE:
                  if( !fill_marquee && !empty_marquee)
                  {
                    empty_marquee = true;
                    marquee_start.first = mx + camera.get_bounds()->x;
                    marquee_start.second = my + camera.get_bounds()->y;
                  }
                  break;
                  default: break;
                }
                interpretTerrain();
              }
              else if(edit_tool == SELECT && view == INTERACTIVE && inter_mode == ENEMIES && enemy_select >= 0 && enemy_select < map.getEnemies()->size())
              {
                pair<int,int> point;
                point.first = mx + camera.get_bounds()->x;
                point.second = my + camera.get_bounds()->y;
                map.getEnemies()->at(enemy_select)->addPoint(point);
              }
              else if(edit_tool == PLUS)
              {
                cycleObjects(true);
              }
            }
            break;
          case SDL_BUTTON_WHEELUP: if(view != TERRAIN && edit_tool == PLUS) { cycleObjects(false); }break;
          case SDL_BUTTON_WHEELDOWN: if(view != TERRAIN && edit_tool == PLUS) { cycleObjects(true); }break;
          default: break;
        }
    }
     else if(event.type == SDL_MOUSEBUTTONUP)
    {
      if(my<0 || my > WINDOW_HEIGHT || mx<0 || mx>WINDOW_WIDTH)
        return;

      switch(event.button.button)
      {
        case SDL_BUTTON_LEFT:
            if(fill_marquee)
            {
              if(my>BUTTONSIZE)
                marqueeTiles(mx,my);

              fill_marquee=false;
            }
            break;
        case SDL_BUTTON_RIGHT:
            if(empty_marquee)
            {
              if(my>BUTTONSIZE)
                marqueeTiles(mx,my);

              empty_marquee=false;
            }
            break;
        default: break;
      }
    }
    /// MOUSE MOVEMENT - includes hovering over buttons and painting with brush or rectangle
    else if (event.type == SDL_MOUSEMOTION)
    {
      /// mouse is not clicked and is within window
      if(!mousedown && mx > 0 && mx < WINDOW_WIDTH && my > 0 && my < WINDOW_HEIGHT)
      {
        // get rid of button highlights
        for(int i=0; i<icons.size(); i++)
          icons[i]->mouseoff();
        for(int i=0; i<tools.size(); i++)
          tools[i]->mouseoff();
        /// mouse is on the toolbar
        if(my < BUTTONSIZE)
        {
          int index = mx / BUTTONSIZE;
          if(index < icons.size() && !icons[index]->isDown())
            icons[index]->mouseover();
          else
          {
            int t = (WINDOW_WIDTH - mx) / BUTTONSIZE;
            if(view == TERRAIN && t < tools.size())
            {
              t = tools.size() - t;
              if(!tools[t-1]->isDown())
                tools[t-1]->mouseover();
            }
            else if(view != TERRAIN)
            {
              icon_select.mouseoff();
              icon_plus.mouseoff();
              icon_toggle_paths.mouseoff();
              // first from the right, plus icon
              if(t < 1 )
              {
                icon_plus.mouseover();
              }
              // second from the right, plus icon
              else if(t < 2 )
              {
                icon_select.mouseover();
              }
              // third from the right, path toggle
              else if(t < 3 )
              {
                icon_toggle_paths.mouseover();
              }
            }
          }
        }
        /// mouse is on the dropdown menu
        else if(my < dropdown_height + BUTTONSIZE &&  mx < options[0]->get_bounds().w)
        {
          // get rid of button highlights
          for(int i=0; i<options.size(); i++)
            options[i]->mouseoff();

          for(int i=0; i<options.size() && my > options[i]->get_bounds().y; i++)
          {
            if(my < options[i]->get_bounds().y + options[i]->get_bounds().h/2)
              options[i]->mouseover();
          }
        }
        /// mouse is somewhere else; close the dropdown menu
        else
        {
          icon_properties.force_up();
          option_menu = false;
        }
      }
      if(view == INTERACTIVE && inter_mode == ENEMIES && edit_tool == SELECT && mousedown)
      {
        if(enemy_select >= 0 && enemy_select < map.getEnemies()->size())
        {
          vector<pair<int, int> >* path = map.getEnemies()->at(enemy_select)->getPath();
          if(path_point_select > 0 && path_point_select < path->size())
          {
            path->at(path_point_select).first = mx + camera.get_bounds()->x;
            path->at(path_point_select).second = my + camera.get_bounds()->y;
          }
        }
      }
    }
    if(view == TERRAIN && tool == BRUSH && !option_menu)
    {
      brushAndErase();
      simplifyTiles();
      interpretTerrain();
    }
  }
  // shift would mean we are using discrete movement
  if(!keystate[SDLK_LSHIFT] && !keystate[SDLK_RSHIFT])
  {
    // movement without scrolling depends on control
    // as well as resizing the map if in TERRAIN view
    if(!keystate[SDLK_LCTRL] && !keystate[SDLK_RCTRL])
    {
      if(keystate[SDLK_RIGHT] || keystate[SDLK_d])
        camera.scroll(RIGHT, 2);
      else if(keystate[SDLK_LEFT] || keystate[SDLK_a])
        camera.scroll(LEFT, 2);
      if(keystate[SDLK_UP] || keystate[SDLK_w])
        camera.scroll(UP, 2);
      else if(keystate[SDLK_DOWN] || keystate[SDLK_s])
        camera.scroll(DOWN, 2);
    }
    // nudge selected background object
    if(view == BACKGROUND)
    {
      if(keystate[SDLK_RIGHT])
        nudgeSelectionBG(RIGHT, 2);
      else if(keystate[SDLK_LEFT])
        nudgeSelectionBG(LEFT, 2);
      if(keystate[SDLK_UP])
        nudgeSelectionBG(UP, 2);
      else if(keystate[SDLK_DOWN])
        nudgeSelectionBG(DOWN, 2);
    }
    else if(view == INTERACTIVE && inter_mode == CATCHERS)
    {
      if(keystate[SDLK_RIGHT])
        nudgeCatcher(RIGHT, 2);
      else if(keystate[SDLK_LEFT])
        nudgeCatcher(LEFT, 2);
      if(keystate[SDLK_UP])
        nudgeCatcher(UP, 2);
      else if(keystate[SDLK_DOWN])
        nudgeCatcher(DOWN, 2);
    }
    else if(view == INTERACTIVE && inter_mode == ENEMIES)
    {
      if(keystate[SDLK_RIGHT] && enemy_select>=0 && enemy_select < map.getEnemies()->size())
        nudgePath(map.getEnemies()->at(enemy_select), RIGHT, 2);
      else if(keystate[SDLK_LEFT] && enemy_select>=0 && enemy_select < map.getEnemies()->size())
        nudgePath(map.getEnemies()->at(enemy_select), LEFT, 2);
      if(keystate[SDLK_UP] && enemy_select>=0 && enemy_select < map.getEnemies()->size())
        nudgePath(map.getEnemies()->at(enemy_select), UP, 2);
      else if(keystate[SDLK_DOWN] && enemy_select>=0 && enemy_select < map.getEnemies()->size())
        nudgePath(map.getEnemies()->at(enemy_select), DOWN, 2);
    }
    else if(view == INTERACTIVE && inter_mode == ITEMS)
    {
      if(keystate[SDLK_RIGHT])
        nudgeItem(RIGHT, 2);
      else if(keystate[SDLK_LEFT])
        nudgeItem(LEFT, 2);
      if(keystate[SDLK_UP])
        nudgeItem(UP, 2);
      else if(keystate[SDLK_DOWN])
        nudgeItem(DOWN, 2);
    }
    // nudge selected foreground object
    else if(view == FOREGROUND)
    {
      if(keystate[SDLK_RIGHT])
        nudgeSelectionFG(RIGHT, 2);
      else if(keystate[SDLK_LEFT])
        nudgeSelectionFG(LEFT, 2);
      if(keystate[SDLK_UP])
        nudgeSelectionFG(UP, 2);
      else if(keystate[SDLK_DOWN])
        nudgeSelectionFG(DOWN, 2);
    }
  }
}

void Editor::run()
{
  cout << "Entering EDITOR mode.\n\n";

  // do any post menu setup here
  stateMachine -> setState(EDITOR);
  SDL_ShowCursor(SDL_ENABLE);

  // if a map isn't already loaded
  if(map.get_Width() == 0 || map.get_Height() == 0)
  {
    // load the default map, custom.map
    cout << "map load defaulted to custom.map\n";
    mapname = "custom";
    if(!loadFile())
    {
      cout << "Aborting editor due to map load failure.\n";
      stateMachine -> setState(MAIN_MENU);
    }
    // reload the editor's list of available objects, enemies, and catchers
  }

  populateObjectBanks();
  interpretTerrain();

  /// main game loop
  while( stateMachine -> getState() == EDITOR )
  {
    // get input
    input();

    // change positions of stuff
    update();

    // draw to the screen buffer
    display();

    // displays the new screen image
    artist -> flip();

    // check how fast the last frame took to create, and delay the
    // next one accordingly.
    // The argument specifies whether we want menu FPS or in-game FPS.
    artist -> framerate_limit(false);
  }
  //SDL_ShowCursor(SDL_DISABLE);
  cout << "Leaving Editor\n";
}

void Editor::validateCamera()
{
  // if the camera is too far left, move it
  if(camera.get_x() < 0)
    camera.set_x(0);
  // if it's too far right, move it
  else if(camera.get_x() + camera.get_w() > map.get_Widthpx())
    camera.set_x(map.get_Widthpx() - camera.get_w());
  // if it's too far up, move it
  if(camera.get_y() < 0  - BUTTONSIZE)
    camera.set_y(0 - BUTTONSIZE);
  // if it's too far down, move it
  else if(camera.get_y() + camera.get_h() > map.get_Heightpx())
    camera.set_y(map.get_Heightpx() - camera.get_h());
}

void Editor::resizeMap(int newWidth, int newHeight)
{
  // size in tiles, not pixels
  int oldWidth = map.get_Width();
  int oldHeight = map.get_Height();
  // minimum map size is the size of the window
  if(newWidth < WINDOW_WIDTH/TILESIZE)
    newWidth = WINDOW_WIDTH/TILESIZE;
  if(newHeight < WINDOW_HEIGHT/TILESIZE)
    newHeight = WINDOW_HEIGHT/TILESIZE;
  // max map size 200 tiles by 200 tiles (for memory conservation)
  if(newHeight > 200)
    newHeight = 200;
  if(newWidth > 200)
    newWidth = 200;

  vector<Tile*>* tiles = map.get_TilesPointer();

  vector<Tile*> tilescopy = map.get_Tiles();

  // if width is shrinking
  if (newWidth < oldWidth)
  {
    for(int k = 0; k < oldHeight; k++)
    {
      // set the corresponding tile in our copy map to -1 (to mark for deletion)
      for (int i = newWidth; i < oldWidth; i++)
      {
        tilescopy[k*oldWidth + i]->set_type(-1);
      }
    }
    // actually delete the tiles set to -1
    for(vector<Tile*>::iterator it = tiles->begin(); it != tiles->end(); it++)
    {
      Tile* tile = *it;
      if(tile->get_type() == -1)
      {
        tiles->erase(it);
        it--;
      }
    }
  }

  // if height is shrinking
  if(newHeight < oldHeight)
  {
    // simply remove the extra tiles (they're on the end)
    tiles->resize(map.get_Width() * newHeight);
  }

  // if width is growing
  if(newWidth > oldWidth)
  {
    // use whichever height value is smaller
    int k;
    if (newHeight < oldHeight)
      k = newHeight;
    else
      k = oldHeight;
    // for each row in the map
    for (; k>0; k--)
    {
      // for each NEW tile in the row
      for(int i = oldWidth; i<newWidth; i++)
      {
        // add a new tile, default type to air
        Tile* tile = new Tile(i,k-1,0);
        tiles->insert( tiles->begin() + (oldWidth * k) + i-oldWidth, 1, tile);
      }
    }
  }

  // if the height is growing
  if(newHeight> oldHeight)
  {
    // for each new row
    for (int k = oldHeight; k<newHeight; k++)
    {
      // for each tile in the new row
      for(int i = 0; i<newWidth; i++)
      {
        // add new tile defaulted to air type
        Tile* tile = new Tile(i,k,0);
        tiles->push_back(tile);
      }
    }
  }

  // update the map's height/width properties
  map.set_Width(newWidth);
  map.set_Height(newHeight);

}

// voodoo logic.
// Tries to figure out what a given solid (square) tile's true (in-game) type is judging by its editor type and the
// types of the tiles touching it. Still a few bugs, just fix it by hand if you see one.
int Editor::interpret(  int tile,
                        int topleft,
                        int top,
                        int topright,
                        int left,
                        int right,
                        int bottomleft,
                        int bottom,
                        int bottomright
                     )
{
  if((top == 2 || top == 33 || top == 4 || top == 5) && (right == 2 || right == 32 || right == 36 || right == 43) && (left == 2 || left==31 || left == 35 || left ==42) && (bottom == 2 || bottom == 30 || bottom == 25 || bottom == 26) && topleft && topleft!=1 && topright && topright!=1 && bottomleft && bottomleft!=1 && bottomright && bottomright!=1)
    tile = 15;
  else if(((top == 0 || top == 30) || top == 1 || ((!topleft || topleft == 1) && (!topright || topright == 1)))&& (right == 2 || right == 5 || right == 32 || right == 18 || right == 5 || right == 29) && (left == 2 || left==31 || left == 19 || left == 4 || left == 28) &&
         (bottom == 2 || bottom == 30 || bottom ==25 || bottom ==26 || bottom == 43) && (bottomleft && bottomleft!= 1) && (bottomright && bottomright != 1))
    tile = 8;
  else if((top == 2 || top == 33 || top == 18 || top == 36 || top == 6 || top == 36) && (right == 0 || right == 1 || right == 31 || ((!topright || topright==1)&& (!bottomright || bottomright == 1))) && (left == 2 || left==31 || left==35 || left == 42 || left == 4) && (bottom == 2 || bottom == 30 || bottom == 11 || bottom == 43 || bottom == 27) && topleft && topleft!=1 && bottomleft && bottomleft!=1)
    tile = 16;
  else if((top == 2 || top == 33 || top == 19 || top == 35 || top == 3 || top == 35) && (right == 2 || right == 32 || right == 36 || right == 43) &&
          (left == 0 || left == 1 || left == 32 || ((!topleft || topleft ==1)&&( !bottomleft || bottomleft == 1))) && (bottom == 2 || bottom == 30 || bottom == 12 || bottom == 42 || bottom == 24 || bottom == 42) && topright && topright!=1 && bottomright && bottomright!=1)
    tile = 14;
  else if((top == 2 || top == 33  || top == 4 || top == 5) && (right == 2 || right == 32 || right == 11 || right == 26 || right == 50 || right == 26) && (left == 2 || left==31 || left == 12 || left == 25 || left == 49 || left == 25) &&
          (bottom == 0 || bottom == 1 || bottom ==33 || ((!bottomleft || bottomleft==1)&& (!bottomright || bottomright == 1))) && topleft && topleft!=1 && topright && topright != 1)
    tile = 22;
  else if((top == 2 || top == 19 || top == 35 || top == 3) && (left == 0 || left == 1 || left == 32 || ((!topleft || topleft ==1)&&( !bottomleft || bottomleft == 1))) && (bottom == 0 || bottom == 1 || bottom ==33 || ((!bottomleft || bottomleft==1)&& (!bottomright || bottomright == 1))) && (right == 2 || right == 11 || right == 50 || right == 26) && topright && topright!=1)
    tile = 21;
  else if((top == 2 || top == 18 || top == 6 || top == 36) && (left== 2 || left == 12 || left == 49 || left == 25) && (bottom == 0 || bottom == 1 || bottom ==33 || ((!bottomleft || bottomleft==1)&& (!bottomright || bottomright == 1))) && (!right || right == 1 || right == 31 || ((!topright || topright==1)&& (!bottomright || bottomright == 1))) && topleft && topleft!=1)
    tile = 23;
  else if(((top == 0 || top == 30) || top == 1 || ((!topleft || topleft == 1)&& (!topright || topright == 1))) && (left == 0 || left == 1 || left == 32 || ((!topleft || topleft ==1)&&( !bottomleft || bottomleft == 1))) && (bottom == 2 || bottom == 24 || bottom == 12 || bottom == 42) && (right == 2 || right == 18 || right == 5 || right == 29) && bottomright && bottomright!=1)
    tile = 7;
  else if(((top == 0 || top == 30) || top == 1 || ((!topleft || topleft == 1)&& (!topright || topright == 1 ))) && (left== 2 || left == 12 || left == 49 || left == 25) && (bottom == 2 || bottom == 27 || bottom == 11 || bottom == 43) && (!right || right==1 || right==31 || ((!bottomright || bottomright==1) && ((!topright || topright == 1 )))) && bottomleft && bottomleft != 1)
    tile = 9;
  else if(left && top && bottom && right && top!=1 && right != 1 && left != 1 && bottom!=1)
  {
    if((bottomright == 0 || bottomright == 1 ||bottomright == 18 || bottomright == 29 || bottomright == 5 || bottomright == 24 || bottomright == 12 || bottomright == 42 || bottomright == 27 || bottomright == 30 ) &&
         (topleft == 0 || topleft == 1 ||topleft == 12 || topleft == 49 || topleft == 25 || topleft == 6 || topleft == 18 || topleft == 36) && bottom != 30 && bottom != 26 && left!=31 && left != 35)
      tile = 41;
    else if((bottomleft == 0 || bottomleft == 1 || bottomleft == 19 || bottomleft == 28 || bottomleft == 4 || bottomleft == 27 || bottomleft == 11 || bottomleft == 43 || bottomleft == 24 || bottomleft == 30 ) &&
            (topright == 0 || topright == 1 || topright == 11 || topright == 50 || topright == 26 || topright == 3 || topright == 19 || topright == 35) && bottom != 30 && bottom != 25 && right!=32 && right != 36)
      tile = 34;
    else if((bottomright == 0 || bottomright == 1 || bottomright == 18 || bottomright == 29 || bottomright == 5 || bottomright == 24 || bottomright == 12 || bottomright == 42 ||  bottomright == 27 || bottomright == 30 ) && bottom != 30 && bottom != 26 && bottom!=24 && bottom!= 12)
      tile = 10;
    else if((topleft == 0 || topleft == 1 || topleft == 12 || topleft == 49 || topleft == 25 || topleft == 6 || topleft == 18 || topleft == 36) && left!=31 && left != 35)
      tile = 20;
    else if((bottomleft == 0 || bottomleft == 1 || bottomleft == 19 || bottomleft == 28 || bottomleft == 4 || bottomleft == 27 || bottomleft == 11 || bottomleft == 43 || bottomleft == 24 || bottomleft == 30 ) && bottom != 30 && bottom != 25 && bottom!= 27 && bottom!= 11)
      tile = 13;
    else if((topright == 0 || topright == 1 || topright == 11 || topright == 50 || topright == 26 || topright == 3 || topright == 19 || topright == 35) && right!=32 && right != 36 && top != 33)
      tile = 17;
  }
  return tile;
}

// converts the tiles in Editor's terrain form to game's terrain form
void Editor::interpretTerrain()
{
  std::vector<Tile*> tiles = map.get_Tiles();

  if(tiles.empty())
    return;

  int current= 0;
  int topleft= 0;
  int top= 0;
  int topright= 0;
  int left= 0;
  int right= 0;
  int bottomleft= 0;
  int bottom= 0;
  int bottomright= 0;

  int width = map.get_Width();
  //int height = map.get_Height();

  for(int i = 0; i< tiles.size(); i++)
  {
    current = tiles[i] -> get_type();

    if(simplify(current) == 2)
    {
      if((i+1)%width != 0 || !i)
      {
        if(i >= width)
          topright = simplify(tiles[i-width+1] -> get_type());
        else
          topright = 2;

        right = simplify(tiles[i+1] -> get_type());

        if(i < tiles.size()-width)
          bottomright = simplify(tiles[i+width+1] -> get_type());
        else
          bottomright = 2;
      }
      else
      {
        right = 2;
        topright = 2;
        bottomright = 2;
      }

      if(i!=0 && i%width != 0)
      {
        if(i >= width)
          topleft = simplify(tiles[i-width-1] -> get_type());
        else
          topleft = 2;

        left = simplify(tiles[i-1] -> get_type());

        if(i < tiles.size()-width)
          bottomleft = simplify(tiles[i+width-1] -> get_type());
        else
          bottomleft = 2;
      }
      else
      {
        topleft = 2;
        left = 2;
        bottomleft = 2;
      }
      if(i >= width)
        top = simplify(tiles[i-width] -> get_type());
      else
        top = 2;

      if(i < tiles.size()-width)
        bottom = simplify(tiles[i+width] -> get_type());
      else
        bottom = 2;
      tiles[i]->set_type ( interpret(current, topleft, top, topright, left, right, bottomleft, bottom, bottomright) );
    }
  }
}

// converts one tile into the editor's terrain tile form
int Editor::simplify(int type)
{
  if(type == 2 || (type>=7 && type<=9) || (type>=14 && type <= 16) || (type>=21 && type <= 23) || type == 34 || type == 41 || type == 10 || type == 13 || type == 17 || type == 20)
    return 2;

  return type;
}

// converts all tiles in a map to the editor's terrain tile form
void Editor::simplifyTiles()
{
  vector<Tile*> tiles = map.get_Tiles();

  if(tiles.empty())
    return;

  for(std::vector<Tile*>::iterator tile = tiles.begin(); tile != tiles.end(); tile++)
  {
    Tile* mytile = *tile;
    int type = mytile->get_type();

    mytile->set_type(simplify(type));
  }
}

void Editor::tileCycle(bool rightClick)
{
    int curs_x = 0;
    int curs_y = 0;

    SDL_GetMouseState(&curs_x, &curs_y);

    curs_x += camera.get_bounds()->x;
    curs_y += camera.get_bounds()->y;

    Tile* tile =  map.get_Tilepx(curs_x, curs_y);

    if(!rightClick)
    {
      switch(tile->get_type())
      {
        case 0: tile->set_type(2); break;
        case 1: tile->set_type(18); break;
        case 2: tile->set_type(1); break;

        case 11: case 12: case 18:
        case 19: tile->set_type(6); break;

        case 3: case 6: case 24: case 27: case 28: case 29: case 49:
        case 50: tile->set_type(5); break;

        case 4: case 5: case 25: case 26: case 35: case 36: case 42:
        case 43: tile->set_type(33); break;

        case 30: case 31: case 32:
        case 33: tile->set_type(0); break;
      }
    }
    else if(rightClick)
    {
      switch(tile->get_type())
      {
        case 0: tile->set_type(2); break;
        case 1: tile->set_type(0); break;
        case 2: tile->set_type(1); break;
        case 11: tile->set_type(12); break;
        case 12: tile->set_type(19); break;
        case 19:tile->set_type(18); break;
        case 18: tile->set_type(11); break;
        case 3: tile->set_type(6); break;
        case 6: tile->set_type(29); break;
        case 29: tile->set_type(50); break;
        case 50: tile->set_type(27); break;
        case 27: tile->set_type(24); break;
        case 24: tile->set_type(49); break;
        case 49: tile->set_type(28); break;
        case 28: tile->set_type(3); break;
        case 4: tile->set_type(5); break;
        case 5: tile->set_type(36); break;
        case 36: tile->set_type(43); break;
        case 43: tile->set_type(26); break;
        case 26: tile->set_type(25); break;
        case 25: tile->set_type(42); break;
        case 42: tile->set_type(35); break;
        case 35: tile->set_type(4); break;
        case 33: tile->set_type(32); break;
        case 32: tile->set_type(30); break;
        case 30: tile->set_type(31); break;
        case 31: tile->set_type(33); break;
      }
    }
}

void Editor::brushAndErase()
{
  int x, y;
  Uint8 mouse = SDL_GetMouseState(&x, &y);

  if(y<BUTTONSIZE || y > WINDOW_HEIGHT || x<0 || x>WINDOW_WIDTH)
    return;

  x += camera.get_bounds()->x;
  y += camera.get_bounds()->y;
  //LMB state
  if (mouse & SDL_BUTTON(SDL_BUTTON_LEFT))
    map.get_Tilepx(x, y)->set_type(2);
  //RMB state
  else if (mouse & SDL_BUTTON(SDL_BUTTON_RIGHT))
    map.get_Tilepx(x, y)->set_type(0);
}

void Editor::marqueeTiles(int x, int y)
{

  x += camera.get_bounds()->x;
  y += camera.get_bounds()->y;
  int sx = marquee_start.first;
  int sy = marquee_start.second;

  cout << "Entering marqueeTiles() with " << sx << "," << sy << " to " << x << "," << y << "\n";

  x = x/TILESIZE;
  y = y/TILESIZE;
  sx = sx/TILESIZE;
  sy = sy/TILESIZE;

  cout << "Tiles are now " << sx << "," << sy << " to " << x << "," << y << "\n";

  if(sx < 0 || sx > map.get_Width() || sy < 0 || sy > map.get_Height()
      || x< 0 || x > map.get_Width() || y<0 || y>map.get_Height())
  {
    cout << "Out of bounds marquee attempt.\n";
    return;
  }
  int fill = -1;

  if(fill_marquee)
    fill = 2;
  else if(empty_marquee)
    fill = 0;
  else
    return;

  simplifyTiles();

  if( x <= sx && y<= sy)
  {
   for(int i = x; i<=sx; i++)
   {
     for(int j = y; j<=sy; j++)
     {
       cout << "[marquee] Changing tile " << i << "," << j << " to " << fill << "\n";
       map.get_Tile(i,j)->set_type(fill);
     }
   }
  }
  else if(x <= sx && y>sy)
  {
   for(int i = x; i<=sx; i++)
   {
     for(int j = sy; j<=y; j++)
     {
       cout << "[marquee] Changing tile " << i << "," << j << " to " << fill << "\n";
       map.get_Tile(i,j)->set_type(fill);
     }
   }
  }
  else if(x > sx && y<= sy)
  {
   for(int i = sx; i<=x; i++)
   {
     for(int j = y; j<=sy; j++)
     {
       cout << "[marquee] Changing tile " << i << "," << j << " to " << fill << "\n";
       map.get_Tile(i,j)->set_type(fill);
     }
   }
  }
  else if(x > sx && y>sy)
  {
   for(int i = sx; i<=x; i++)
   {
     for(int j = sy; j<=y; j++)
     {
       cout << "[marquee] Changing tile " << i << "," << j << " to " << fill << "\n";
       map.get_Tile(i,j)->set_type(fill);
     }
   }
  }
  interpretTerrain();
}

void Editor::drawMarquee(bool plus)
{
  int x, y;
  Uint8 mouse = SDL_GetMouseState(&x, &y);

  x += camera.get_bounds()->x;
  y += camera.get_bounds()->y;
  int sx = marquee_start.first;
  int sy = marquee_start.second;

  x = x/TILESIZE * TILESIZE;
  y = y/TILESIZE * TILESIZE;
  sx = sx/TILESIZE * TILESIZE;
  sy = sy/TILESIZE * TILESIZE;

  if(sx < 0 || sx > map.get_Widthpx() || sy < 0 || sy > map.get_Heightpx()
      || x< 0 || x > map.get_Widthpx() || y<0 || y>map.get_Heightpx())
  {
    return;
  }

  if( x <= sx && y<= sy)
  {
    for(int i = x; i<=sx; i+=TILESIZE)
    {
      for(int j = y; j<=sy; j+=TILESIZE)
      {
        if(plus)
          tile_plus.display(screen, false, i - camera.get_x(), j - camera.get_y());
        else
          tile_minus.display(screen, false, i- camera.get_x(), j - camera.get_y());
      }
    }
  }
  else if(x <= sx && y>sy)
  {
    for(int i = x; i<=sx; i+=TILESIZE)
    {
     for(int j = sy; j<=y; j+=TILESIZE)
     {
        if(plus)
          tile_plus.display(screen, false, i- camera.get_x(), j - camera.get_y());
        else
          tile_minus.display(screen, false, i- camera.get_x(), j - camera.get_y());
      }
    }
  }
  else if(x > sx && y<= sy)
  {
   for(int i = sx; i<=x; i+=TILESIZE)
   {
      for(int j = y; j<=sy; j+=TILESIZE)
      {
        if(plus)
          tile_plus.display(screen, false, i- camera.get_x(), j - camera.get_y());
        else
          tile_minus.display(screen, false, i- camera.get_x(), j - camera.get_y());
      }
    }
  }
  else if(x > sx && y>sy)
  {
    for(int i = sx; i<=x; i+=TILESIZE)
    {
      for(int j = sy; j<=y; j+=TILESIZE)
      {
        if(plus)
          tile_plus.display(screen, false, i- camera.get_x(), j - camera.get_y());
        else
          tile_minus.display(screen, false, i- camera.get_x(), j- camera.get_y());
      }
    }
  }
}

double getDistance(int x1, int y1, int x2, int y2)
{
  double result =  sqrt(((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1)));
  //cout << "Distance between " << x1 << "," << y1 << " and " << x2 << "," << y2 << " is " << result << endl;
  return result;
}

bool Editor::selectPathPoint(int x, int y)
{
  int current_enemy=0;
  int last_enemy = map.getEnemies()->size() -1;

  if(!show_paths)
  {
    current_enemy = enemy_select;
    last_enemy = enemy_select + 1;
  }

  for(;current_enemy<last_enemy && current_enemy < map.getEnemies()->size();current_enemy++)
  {
    if(current_enemy >= 0 && current_enemy < map.getEnemies()->size())
    {
      int first = 0;
      int last = 0;

      vector<pair<int, int> >* path = map.getEnemies()->at(current_enemy)->getPath();

      if(path->size() <= 1)
      {
        path_point_select = -1;
        continue;
        //return false;
      }

      if(path_point_select > 0 && path_point_select < path->size() -1 && path->size() > 2 )
        first = path_point_select + 1;
      else
        first = 1;

      path_point_select = -1;

      if(getDistance(x, y, path->at(first).first, path->at(first).second) <= NODERADIUS+3)
      {
        path_point_select = first;
        enemy_select = current_enemy;
        return true;
      }

      last = first;
      first += 1;

      if(first >= path->size())
        first = 1;

      for(int i = first; i != last; i++)
      {
        if(getDistance(x, y, path->at(i).first, path->at(i).second) <= NODERADIUS+3)
        {
          path_point_select = i;
          enemy_select = current_enemy;
          return true;
        }

        if(i >= path->size() -1)
          i = 0;
      }
    }
  }
  path_point_select = -1;
  return false;
}

void Editor::selectObject()
{
  vector<Object*>* distants;
  vector<Object*>* objects;
  vector<Catcher*>* catchers;
  vector<Enemy*>* enemies;
  vector<Item*>* items;

  if(view == BACKGROUND)
  {
    objects = map.getObjects("background");
    distants = map.getObjects("distance");
  }
  else if(view == FOREGROUND)
    objects =  map.getObjects("foreground");
  else if(view == INTERACTIVE && inter_mode == CATCHERS)
    catchers = map.getCatchers();
  else if(view == INTERACTIVE && inter_mode == ENEMIES)
    enemies = map.getEnemies();
  else if(view == INTERACTIVE && inter_mode == ITEMS)
    items = map.getItems();


  if((view == BACKGROUND || view == FOREGROUND) && (objects == NULL || objects->empty()))
  {
    if( view == FOREGROUND || distants == NULL || distants->empty())
    {
      cout << "cannot select; no objects to select " <<endl;
      return;
    }
  }
  else if((view == INTERACTIVE && inter_mode == CATCHERS) && (catchers == NULL || catchers->empty()))
  {
    cout << "cannot select; no catchers to select " <<endl;
    return;
  }
  else if((view == INTERACTIVE && inter_mode == ENEMIES) && (enemies == NULL || enemies->empty()))
  {
    cout << "cannot select; no enemies to select " <<endl;
    return;
  }
  else if((view == INTERACTIVE && inter_mode == ITEMS) && (items == NULL || items->empty()))
  {
    cout << "cannot select; no items to select " <<endl;
    return;
  }

  int curs_x = 0;
  int curs_y = 0;

  SDL_GetMouseState(&curs_x, &curs_y);

  curs_x += camera.get_x();
  curs_y += camera.get_y();

  int first, last;
  int dist_first = 0;
  if(view != BACKGROUND || (objects != NULL && !objects->empty()))
  {
    if(view == BACKGROUND)
    {
      if(bg_select >= 0 && bg_select < objects->size() - 1 && objects->size() > 1)
        first = bg_select + 1;
      else
        first = 0;

      bg_select = -1;

      if(distants != NULL && !distants->empty())
      {
        if(dist_select >= 0 && dist_select < distants->size() - 1 && distants->size() > 1)
          dist_first = dist_select + 1;

        dist_select = -1;
      }
    }
    else if(view == INTERACTIVE && inter_mode == CATCHERS)
    {
      if(catcher_select >= 0 && catcher_select < catchers->size() - 1 && catchers->size() > 1)
        first = catcher_select + 1;
      else
        first = 0;

      catcher_select = -1;
    }
    else if(view == INTERACTIVE && inter_mode == ENEMIES)
    {
      if(enemy_select >= 0 && enemy_select < enemies->size() - 1 && enemies->size() > 1)
        first = enemy_select + 1;
      else
        first = 0;

      enemy_select = -1;
    }
    else if(view == INTERACTIVE && inter_mode == ITEMS)
    {
      if(item_select >= 0 && item_select < items->size() - 1 && items->size() > 1)
        first = item_select + 1;
      else
        first = 0;

      item_select = -1;
    }
    else if(view == FOREGROUND)
    {
      if(fg_select >= 0 && fg_select < objects->size() - 1 && objects->size() > 1)
        first = fg_select + 1;
      else
        first = 0;

      fg_select = -1;
    }

    SDL_Rect rect;
    if(view == BACKGROUND || view == FOREGROUND)
    {
      rect = *objects->at(first)->get_bounds();
      int speed = objects->at(first)->get_scrollspeed();
      if( speed > 1)
      {
        rect.x += camera.get_x()*(speed-1)/speed;
        rect.y += camera.get_y()*(speed-1)/speed;
      }
    }
    else if (view == INTERACTIVE && inter_mode == CATCHERS)
      rect = *catchers->at(first)->get_bounds();
    else if (view == INTERACTIVE && inter_mode == ENEMIES)
      rect = *enemies->at(first)->get_bounds();
    else if (view == INTERACTIVE && inter_mode == ITEMS)
      rect = *items->at(first)->get_bounds();
    // check the first one
    if(rect.x <= curs_x && (rect.x + rect.w) >= curs_x && rect.y <= curs_y && (rect.y + rect.h) >= curs_y)
    {
      if(view == BACKGROUND)
      {
        bg_select = first;
        cout << "Selected background object[" << bg_select << "]\n";
      }
      else if (view == INTERACTIVE && inter_mode == CATCHERS)
      {
        catcher_select = first;
        cout << "Selected catcher[" << catcher_select << "]\n";
      }
      else if (view == INTERACTIVE && inter_mode == ENEMIES)
      {
        enemy_select = first;
        cout << "Selected enemy[" << enemy_select << "]\n";
      }
      else if (view == INTERACTIVE && inter_mode == ITEMS)
      {
        item_select = first;
        cout << "Selected item[" << item_select << "]\n";
      }
      else if (view == FOREGROUND)
      {
        fg_select = first;
        cout << "Selected foreground object[" << fg_select << "]\n";
      }

      //found it the easy way
      return;
    }

    last = first;
    first += 1;

    if((view == BACKGROUND || view == FOREGROUND) && first >= objects->size())
      first = 0;
    else if(view == INTERACTIVE && inter_mode == CATCHERS && first >= catchers->size())
      first = 0;
    else if(view == INTERACTIVE && inter_mode == ENEMIES && first >= enemies->size())
      first = 0;
    else if(view == INTERACTIVE && inter_mode == ITEMS && first >= items->size())
      first = 0;

    for(int i = first; i != last; i++)
    {
      if(view == BACKGROUND || view == FOREGROUND)
      {
        rect = *objects->at(i)->get_bounds();
        int speed = objects->at(i)->get_scrollspeed();
        if( speed > 1)
        {
          rect.x += camera.get_x()*(speed-1)/speed;
          rect.y += camera.get_y()*(speed-1)/speed;
        }
      }
      else if(view == INTERACTIVE && inter_mode == CATCHERS)
        rect = *catchers->at(i)->get_bounds();
      else if(view == INTERACTIVE && inter_mode == ENEMIES)
        rect = *enemies->at(i)->get_bounds();
      else if(view == INTERACTIVE && inter_mode == ITEMS)
        rect = *items->at(i)->get_bounds();

      if(rect.x <= curs_x && (rect.x + rect.w) >= curs_x && rect.y <= curs_y && (rect.y + rect.h) >= curs_y)
      {
        if(view == BACKGROUND)
        {
          bg_select = i;
          cout << "Selected background object[" << bg_select << "]\n";
        }
        else if(view == INTERACTIVE && inter_mode == CATCHERS)
        {
          catcher_select = i;
          cout << "Selected catcher[" << catcher_select << "]\n";
        }
        else if(view == INTERACTIVE && inter_mode == ENEMIES)
        {
          enemy_select = i;
          cout << "Selected enemy[" << enemy_select << "]\n";
        }
        else if(view == INTERACTIVE && inter_mode == ITEMS)
        {
          item_select = i;
          cout << "Selected items[" << item_select << "]\n";
          return;
        }
        else if (view == FOREGROUND)
        {
          fg_select = i;
          cout << "Selected foreground object[" << fg_select << "]\n";
        }
        return;
      }

      if((view == BACKGROUND || view == FOREGROUND) && i == objects -> size() -1 && objects->size() > 1)
        i = -1;
      if(view == INTERACTIVE && inter_mode == CATCHERS && i == catchers -> size() -1 && catchers->size() > 1)
        i = -1;
      if(view == INTERACTIVE && inter_mode == ENEMIES && i == enemies -> size() -1 && enemies->size() > 1)
        i = -1;
      if(view == INTERACTIVE && inter_mode == ITEMS && i == items -> size() -1 && items->size() > 1)
        i = -1;
    }
  }
  if(view == BACKGROUND)
  {
    if(distants == NULL || distants->empty())
    {
      cout << "cannot select; no distant objects to select " <<endl;
      return;
    }
    SDL_Rect rect;
    rect = *distants->at(dist_first)->get_bounds();
    rect.x += camera.get_x()*3/4;
    rect.y += camera.get_y()*3/4;

    // check the first one
    if(rect.x <= curs_x && (rect.x + rect.w) >= curs_x && rect.y <= curs_y && (rect.y + rect.h) >= curs_y)
    {
      dist_select = dist_first;
      cout << "Selected distant object[" << dist_select << "]\n";
      //found it the easy way
      return;
    }

    last = dist_first;
    first = dist_first + 1;

    if(first >= distants->size())
      first = 0;

    for(int i = first; i != last; i++)
    {
      rect = *distants->at(i)->get_bounds();
      rect.x += camera.get_x()*3/4;
      rect.y += camera.get_y()*3/4;
      if(rect.x <= curs_x && (rect.x + rect.w) >= curs_x && rect.y <= curs_y && (rect.y + rect.h) >= curs_y)
      {
        dist_select = i;
        cout << "Selected distant object[" << dist_select << "]\n";
        return;
      }
      if(i >= distants -> size() -1 && distants->size() > 1)
        i = -1;
    }
    dist_select = -1;
  }
}

void Editor::removeSelected()
{
  if(view == BACKGROUND || view == FOREGROUND)
  {
    string layer;

    if(view == BACKGROUND)
      layer = "background";
    else if(view == FOREGROUND)
      layer = "foreground";

    vector<Object*>* objects = map.getObjects(layer);

    if(objects != NULL && layer == "background" && bg_select >= 0 && bg_select < objects->size())
    {
      objects->erase(objects->begin() + bg_select);
      bg_select = -1;
    }
    else if(layer == "background" && dist_select >= 0)
    {
      objects = map.getObjects("distance");
      if(objects == NULL || dist_select >= objects->size())
        return;

        objects->erase(objects->begin() + dist_select);
        dist_select = -1;
    }
    if(layer == "foreground" && fg_select >= 0 && fg_select < objects->size())
    {
      objects->erase(objects->begin() + fg_select);
      fg_select = -1;
    }
  }
  else if(view == INTERACTIVE && inter_mode == CATCHERS)
  {
    vector<Catcher*>* catchers = map.getCatchers();
    if(catchers == NULL || catchers->empty())
    {
      cout << "Cannot remove; no catchers in map.\n";
      return;
    }
    if(catcher_select >= 0 && catcher_select < catchers->size())
    {
      catchers->erase(catchers->begin() + catcher_select);
      catcher_select = -1;
    }
  }
  else if(view == INTERACTIVE && inter_mode == ITEMS)
  {
    vector<Item*>* items = map.getItems();
    if(items == NULL || items->empty())
    {
      cout << "Cannot remove; no items in map.\n";
      return;
    }
    if(item_select >= 0 && item_select < items->size())
    {
      items->erase(items->begin() + item_select);
      item_select = -1;
    }
  }
  else if(view == INTERACTIVE && inter_mode == ENEMIES)
  {
    vector<Enemy*>* enemies = map.getEnemies();
    if(enemies == NULL || enemies->empty())
    {
      cout << "Cannot remove; no enemies in map.\n";
      return;
    }
    if(enemy_select >= 0 && enemy_select < enemies->size())
    {
      vector<pair<int, int> >* path = enemies->at(enemy_select)->getPath();
      if(path_point_select>0 && path_point_select < path->size())
      {
        path->erase(path->begin() + path_point_select);
        path_point_select = -1;
      }
      else
      {
        enemies->erase(enemies->begin() + enemy_select);
        enemy_select = -1;
      }
    }
  }
}

void Editor::gridBasedNudge(Direction dir)
{
  switch(view)
  {
    case BACKGROUND: nudgeSelectionBG(dir, 0);
                     break;
    case INTERACTIVE: if(inter_mode == CATCHERS) { nudgeCatcher(dir, 0); }
                      else if(inter_mode == ENEMIES && enemy_select>=0 && enemy_select < map.getEnemies()->size())
                      {
                        nudgePath(map.getEnemies()->at(enemy_select), dir, 0);
                      }
                      else if (inter_mode == ITEMS) { nudgeItem(dir, 0); } break;
    case FOREGROUND: nudgeSelectionFG(dir, 0);break;
    default: break;
  }
}

void Editor::nudgeItem(Direction dir, int amount)
{
  vector<Item*>* items = map.getItems();

  if(items == NULL || items->empty() || item_select < 0 || item_select >= items->size())
  {
    cout << "cannot nudge, invalid item_select.\n "<<endl;
    return;
  }

  SDL_Rect* bounds = items->at(item_select)->get_bounds();

  nudgeSelection(bounds, dir, amount);
}

void Editor::nudgeCatcher(Direction dir, int amount)
{
  vector<Catcher*>* catchers = map.getCatchers();

  if(catchers == NULL || catchers->empty() || catcher_select < 0 || catcher_select >= catchers->size())
  {
    cout << "cannot nudge, invalid catcher_select.\n "<<endl;
    return;
  }

  SDL_Rect* bounds = catchers->at(catcher_select)->get_bounds();

  nudgeSelection(bounds, dir, amount);
}

void Editor::nudgeSelectionBG(Direction dir, int amount)
{
  vector<Object*>* objects = map.getObjects("background");

  if(objects == NULL || objects->empty() || bg_select < 0 || bg_select >= objects->size())
  {
    objects = map.getObjects("distance");

    if(objects == NULL || objects->empty() || dist_select < 0 || dist_select >= objects->size())
    {
      cout << "cannot nudge, object not in distance layer "<< endl;
      return;
    }

    SDL_Rect* bounds = objects->at(dist_select)->get_bounds();

    nudgeSelection(bounds, dir, amount);

    return;
  }

  SDL_Rect* bounds = objects->at(bg_select)->get_bounds();

  nudgeSelection(bounds, dir, amount);
}

void Editor::nudgeSelectionFG(Direction dir, int amount)
{
  vector<Object*>* objects = map.getObjects("foreground");

  if(objects == NULL || objects->empty() || fg_select < 0 || fg_select >= objects->size())
  {
    cout << "cannot nudge, object not in background layer "<<endl;
    return;
  }

  SDL_Rect* bounds = objects->at(fg_select)->get_bounds();
  nudgeSelection(bounds, dir, amount);
}

void Editor::nudgePath(Enemy* enem, Direction dir, int amount)
{
  if(amount<=0)
  {
    switch (dir)
    {
      case LEFT: amount = enem->get_bounds()->x % TILESIZE; break;
      case RIGHT: amount = TILESIZE - (enem->get_bounds()->x % TILESIZE); break;
      case UP: amount = enem->get_bounds()->y % TILESIZE; break;
      case DOWN: amount = TILESIZE - (enem->get_bounds()->y % TILESIZE); break;
    }
  if(!amount)
    amount = TILESIZE;
  }
  nudgeSelection(enem->get_bounds(), dir, amount);
  vector<pair<int, int> >* path = enem->getPath();
  switch(dir)
  {
    case UP: amount = enem->get_bounds()->y + (enem->get_bounds()->h/2) - path->at(0).second; break;
    case DOWN: amount = path->at(0).second - (enem->get_bounds()->y + (enem->get_bounds()->h/2)); break;
    case LEFT: amount = enem->get_bounds()->x + (enem->get_bounds()->w/2) - path->at(0).first; break;
    case RIGHT: amount = path->at(0).first - (enem->get_bounds()->x + (enem->get_bounds()->w/2)); break;
  }
  amount = abs(amount);
  for(vector<pair<int, int> >::iterator it = path->begin(); it!= path->end(); it++)
  {
    switch(dir)
    {
      case UP: it->second -= amount; break;
      case DOWN: it->second += amount; break;
      case LEFT: it->first -= amount; break;
      case RIGHT: it->first += amount; break;
    }
  }
}

void Editor::nudgeSelection(SDL_Rect* bounds, Direction dir, int amount)
{
  if(amount<=0)
  {
    switch (dir)
    {
      case LEFT: amount = bounds->x % TILESIZE; break;
      case RIGHT: amount = TILESIZE - (bounds->x % TILESIZE); break;
      case UP: amount = bounds->y % TILESIZE; break;
      case DOWN: amount = TILESIZE - (bounds->y % TILESIZE); break;
    }
    if(!amount)
      amount = TILESIZE;
  }

  switch(dir)
  {
    case UP:    if(bounds->y > amount)
                  bounds->y -= amount;
                else
                  bounds->y = 0;
                break;
    case DOWN:  bounds->y += amount; break;
    case LEFT:  if(bounds->x > amount)
                  bounds->x -= amount;
                else
                  bounds->x = 0;
                break;
    case RIGHT: bounds->x += amount; break;
  }

  if(bounds->w != NULL)
    if(bounds->x + bounds->w > map.get_Widthpx())
      bounds->x = map.get_Widthpx() - bounds->w;

  if(bounds->h != NULL)
    if(bounds->y + bounds->h > map.get_Heightpx())
      bounds->y = map.get_Heightpx() - bounds->h;
}

void Editor::populateObjectBanks()
{
  // vecotr of string filenames
  objectFiles.clear();

  // vectors of these objects
  objectBank.clear();
  enemyBank.clear();
  itemBank.clear();
  catcherBank.clear();

  ifstream index("data/editor_index");
  if(!index)
  {
    cout << "> FAILED to open data/editor_index ... no objects loaded to bank.\n";
    return;
  }
  cout << "> Reading editor object index file... \n";
  string line;
  string theme = "";
  string maptheme = map.get_theme();
  while(index >> line)
  {
    if(line != "" && line != " " && line != "  " && line[0] != ';')
    {
      if(line[0] == '<' && line[1] != '/')
      {
        theme = line.substr(1,line.find_first_of(">")-1);
        // add to our list of available themes
        themes.push_back(theme);
      }
      else if(line[0] == '<' && line[1] == '/')
        theme = "";
      else if(theme == "" || theme == maptheme)
        objectFiles.push_back(line);
    }
  }

  index.close();

  for(int i = 0; i < objectFiles.size(); i++)
  {
    string directory;
    int dotposition = objectFiles[i].find_first_of(".");
    string extension = objectFiles[i].substr(dotposition);
    if( extension == ".obj")
    {
      // unsafe, TODO: throw/catch initialization and image set errors
      objectBank.push_back(new Object(0, 0, objectFiles[i].substr(0, dotposition)));
      if(objectBank.empty() || !objectBank[objectBank.size()-1]->load_successful())
      {
        objectBank.pop_back();
        objectFiles.erase(objectFiles.begin() + i);
        i--;
      }
      else
        objectBank[objectBank.size()-1]->set_image(*artist);
    }
    else if(extension == ".ct")
    {
      // unsafe, TODO: throw/catch initialization and image set errors
      catcherBank.push_back(new Catcher(0, 0, objectFiles[i].substr(0, dotposition)));
      if(catcherBank.empty() || !catcherBank[catcherBank.size()-1]->load_successful())
      {
        catcherBank.pop_back();
        objectFiles.erase(objectFiles.begin() + i);
        i--;
      }
      else
        catcherBank[catcherBank.size()-1]->set_image(*artist);
    }
    else if(extension == ".enemy")
    {
      //cout << "Attempting to load an enemy...\n";
      // unsafe, TODO: throw/catch initialization and image set errors
      enemyBank.push_back(new Enemy(objectFiles[i].substr(0, dotposition)));
      if(enemyBank.empty() || !enemyBank[enemyBank.size()-1]->load_successful())
      {
        enemyBank.pop_back();
        objectFiles.erase(objectFiles.begin() + i);
        i--;
      }
      else
        enemyBank[enemyBank.size()-1]->set_image(*artist);
    }
    else if(extension == ".item")
    {
      cout << "Attempting to load an item...\n";
      // unsafe, TODO: throw/catch initialization and image set errors
      itemBank.push_back(new Item(objectFiles[i].substr(0, dotposition)));
      if(itemBank.empty() || !itemBank[itemBank.size()-1]->load_successful())
      {
        itemBank.pop_back();
        objectFiles.erase(objectFiles.begin() + i);
        i--;
      }
      else
        itemBank[itemBank.size()-1]->set_image(*artist);
    }
    else
    {
      objectFiles.erase(objectFiles.begin() + i);
      i--;
    }
  }
}

void Editor::drawPath(int index, Uint32 color)
{
  vector<pair<int, int> >* path = map.getEnemies()->at(index)->getPath();
  if(path->size()>1)
  {
    vector<pair<int, int> >::iterator it = path->begin();
    vector<pair<int, int> >::iterator next = path->begin();
    SDL_Rect* bds = camera.get_bounds();
    for(++next; next!= path->end(); it++, next++)
    {
      lineColor(screen, it->first - bds->x, it->second - bds->y, next->first - bds->x, next->second - bds->y, color);
      circleColor(screen, next->first - bds->x, next->second - bds->y, NODERADIUS, color);
    }
    if(index == enemy_select && path_point_select > 0 && path_point_select < path->size())
    {
      filledCircleColor(screen, path->at(path_point_select).first - bds->x, path->at(path_point_select).second - bds->y, NODERADIUS, color);
    }
  }
}
