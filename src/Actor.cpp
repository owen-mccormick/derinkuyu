#include <libtcod.hpp>
#include "Actor.hpp"
#include "Map.hpp"

Actor::Actor(int x, int y, int ch, const TCOD_ColorRGB& fg, const TCOD_ColorRGB& bg) : x(x), y(y), ch(ch), fg(fg), bg(bg) {}

// void Actor::act(Map* map) {}

void Actor::render(tcod::Console& console, int cursorX, int cursorY, int displaywidth, int displayheight) const {
  render(console, cursorX, cursorY, displaywidth, displayheight, false);
}

void Actor::render(tcod::Console& console, int cursorX, int cursorY, int displaywidth, int displayheight, bool water) const {
  TCOD_console_put_char_ex(console.get(), x, y - cursorY + displayheight / 2, ch, fg, water ? TCOD_ColorRGB{0, 0, 255} : bg);
}

void Actor::moveTo(Map* map, int x, int y) {
  map->deregisterActorPose(this->x, this->y);
  this->x = x;
  this->y = y;
  map->registerActorPose(x, y);
}

void Actor::moveTo(Map* map, std::pair<int, int> xy) { moveTo(map, xy.first, xy.second); }

int Actor::getX() { return x; }
int Actor::getY() { return y; }
