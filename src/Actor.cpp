#include <libtcod.hpp>
#include "Actor.hpp"

Actor::Actor(int x, int y, std::string ch, const tcod::ColorRGB &col) : x(x), y(y), ch(ch), col(col) {}

void Actor::render(tcod::Console& console, int playerx, int playery, int displaywidth, int displayheight) const {
  tcod::print(console, {x, y - playery + displayheight / 2}, ch, col, std::nullopt);
}
