#pragma once

#include <libtcod.hpp>
#include "Map.hpp"

class Actor { 
  public:
    char ch;
    TCOD_ColorRGB fg; // foreground and background colors
    Actor(int x, int y, char ch, const TCOD_ColorRGB &fg);
    // virtual void act(Map* map);
    void render(tcod::Console& console, int cursorX, int cursorY, int displayWidth, int displayHeight) const;
    void render(tcod::Console& console, int cursorX, int cursorY, int displayWidth, int displayHeight, bool water) const;
    void moveTo(Map* map, int x, int y);
    void moveTo(Map* map, std::pair<int, int> xy);
    int getX();
    int getY();
  private:
    int x,y; // position on map
};
