#include <libtcod.hpp>
#include "Actor.hpp"
#include "Map.hpp"

Actor::Actor(int x, int y, char ch, const TCOD_ColorRGB& fg) : x(x), y(y), ch(ch), fg(fg) {}

void Actor::render(tcod::Console& console, int cursorX, int cursorY, int displayWidth, int displayHeight) const {
  render(console, cursorX, cursorY, displayWidth, displayHeight, false);
}

void Actor::render(tcod::Console& console, int cursorX, int cursorY, int displayWidth, int displayHeight, bool water) const {
  // Changes background based on depth and water
  // The 'if' is necessary because ?: can't choose between a color and nullopt
  if (water) {
    tcod::print(console, {x, y - cursorY + displayHeight / 2}, std::string(1, ch), fg, TCOD_ColorRGB{0, 0, 125});
  } else {
    tcod::print(console, {x, y - cursorY + displayHeight / 2}, std::string(1, ch), fg, std::nullopt);
  }
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
