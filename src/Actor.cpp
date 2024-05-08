#include <libtcod.hpp>
#include "Actor.hpp"
#include "Map.hpp"

Actor::Actor(int x, int y, int ch, const TCOD_ColorRGB& fg, const TCOD_ColorRGB& bg) : x(x), y(y), ch(ch), fg(fg), bg(bg) {}

// void Actor::act(Map* map) {}

void Actor::render(tcod::Console& console, int cursorX, int cursorY, int displaywidth, int displayheight) const {
  TCOD_console_put_char_ex(console.get(), x, y - cursorY + displayheight / 2, ch, fg, bg);
}
