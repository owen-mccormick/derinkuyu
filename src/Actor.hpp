#pragma once

#include <libtcod.hpp>

class Actor { 
  public:
    Actor(int x, int y, std::string ch, const tcod::ColorRGB &col);
    int x,y; // position on map
    std::string ch; // ascii code
    tcod::ColorRGB col; // color
    Actor(int x, int y, int ch, const tcod::ColorRGB &col);
    void render(tcod::Console& console, int playerx, int playery, int displaywidth, int displayheight) const;
};
