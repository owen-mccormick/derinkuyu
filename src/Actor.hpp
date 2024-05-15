#pragma once

#include <libtcod.hpp>
#include "Map.hpp"

class Actor { 
  public:
    int ch; // ascii code
    TCOD_ColorRGB fg, bg; // foreground and background colors
    Actor(int x, int y, int ch, const TCOD_ColorRGB &fg, const TCOD_ColorRGB &bg);
    // virtual void act(Map* map);
    void render(tcod::Console& console, int cursorX, int cursorY, int displaywidth, int displayheight) const;
    void render(tcod::Console& console, int cursorX, int cursorY, int displaywidth, int displayheight, bool water) const;
    void moveTo(Map* map, int x, int y);
    void moveTo(Map* map, std::pair<int, int> xy);
    int getX();
    int getY();
  private:
    int x,y; // position on map
};
