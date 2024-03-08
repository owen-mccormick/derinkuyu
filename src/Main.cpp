#include <iostream>
#include <SDL.h>
#include <libtcod.hpp>
#include <libtcod/timer.hpp>
#include "Map.hpp"
#include "Actor.hpp"

int main(int argc, char* argv[]) {
  static constexpr int WIDTH = 50;
  static constexpr int HEIGHT = 60;
  static constexpr int DISPLAYWIDTH = 50;
  static constexpr int DISPLAYHEIGHT = 30;
  auto console = tcod::Console{DISPLAYWIDTH, DISPLAYHEIGHT};
  auto tileset = tcod::load_tilesheet("data/fonts/dejavu8x8_gs_tc.png", {32, 8}, tcod::CHARMAP_TCOD);
  auto params = TCOD_ContextParams{};
  params.tcod_version = TCOD_COMPILEDVERSION;  // This is required.
  params.console = console.get();  // Derive the window size from the console size.
  params.tileset = tileset.get();
  params.window_title = "...";
  params.sdl_window_flags = SDL_WINDOW_FULLSCREEN;
  params.vsync = true;
  params.argc = argc;  // This allows some user-control of the context.
  params.argv = argv;

  int desiredFPS = 30;
  auto timer = tcod::Timer();
  bool credits = false;

  auto context = tcod::Context(params);

  // Player, map, and actors
  
  Map* map = new Map(WIDTH, HEIGHT, DISPLAYWIDTH, DISPLAYHEIGHT);
  std::vector<Actor*> actors;
  std::pair<int, int> playerPos = map->placePlayer();
  Actor* player = new Actor(playerPos.first, playerPos.second, "@", tcod::ColorRGB(255, 255, 255));
  actors.push_back(player);

  while (true) {
    float deltaTime = timer.sync(desiredFPS);
    TCOD_console_clear(console.get());
    if (!credits) {
      tcod::print(console, {DISPLAYWIDTH / 50, DISPLAYHEIGHT / 3}, "Derinkuyu\nA subterranean city-building game\nBy Owen McCormick", std::nullopt, std::nullopt);
      // credits = TCOD_console_credits_render_ex(console.get(), DISPLAYWIDTH / 10, DISPLAYHEIGHT / 3 + 10, false, deltaTime * 3);
      credits = true;
    } else {
      map->render(console, player->x, player->y);
      for (auto actor : actors) {
        actor->render(console, player->x, player->y, DISPLAYWIDTH, DISPLAYHEIGHT);
      }
      // Water density display
      tcod::print(console, {0, 0}, std::to_string(map->getWater(player->x, player->y)), std::nullopt, std::nullopt);
    }

    context.present(console);  // Updates the visible display.
    SDL_Event event;

    // Gravity
    if (map->isWalkable(player->x, player->y + 1) && player->y < HEIGHT - 1) player->y++;
    map->tick();
  
    // SDL_WaitEvent(nullptr);  // Optional, sleep until events are available.
    while (SDL_PollEvent(&event)) {
      context.convert_event_coordinates(event);  // Optional, converts pixel coordinates into tile coordinates.
      switch (event.type) {
        case SDL_KEYDOWN:
          switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_KP_8: {
              if (map->isWalkable(player->x, player->y - 1) && map->areCoordsValid(player->x, player->y - 1)) player->y--;
              break;
            }  
            case SDL_SCANCODE_KP_2: {
              if (map->isWalkable(player->x, player->y + 1) && map->areCoordsValid(player->x, player->y + 1)) player->y++;
              break;
            }
            case SDL_SCANCODE_KP_4: {
              if (map->isWalkable(player->x - 1, player->y) && map->areCoordsValid(player->x - 1, player->y)) player->x--;
              break;
            }
            case SDL_SCANCODE_KP_6: {
              if (map->isWalkable(player->x + 1, player->y) && map->areCoordsValid(player->x + 1, player->y)) player->x++;
              break;
            }
            case SDL_SCANCODE_KP_7: {
              if (map->isWalkable(player->x - 1, player->y - 1) && map->areCoordsValid(player->x - 1, player->y - 1)) {
                player->x--;
                player->y--;
              }
              break;
            }
            case SDL_SCANCODE_KP_9: {
              if (map->isWalkable(player->x + 1, player->y - 1) && map->areCoordsValid(player->x + 1, player->y - 1)) {
                player->x++;
                player->y--;
              }
              break;
            }
            case SDL_SCANCODE_W: {
              if (map->areCoordsValid(player->x, player->y - 1)) map->setWalkable(player->x, player->y - 1, true);
              break;
            }
            case SDL_SCANCODE_A: {
              if (map->areCoordsValid(player->x - 1, player->y)) map->setWalkable(player->x - 1, player->y, true);
              break;
            }
            case SDL_SCANCODE_S: {
              if (map->areCoordsValid(player->x, player->y + 1)) map->setWalkable(player->x, player->y + 1, true);
              break;
            }
            case SDL_SCANCODE_D: {
              if (map->areCoordsValid(player->x + 1, player->y)) map->setWalkable(player-> x + 1, player->y, true);
              break;
            }
            case SDL_SCANCODE_Q: {
              if (map->areCoordsValid(player->x - 1, player->y - 1)) map->setWalkable(player->x - 1, player->y - 1, true);
              break;
            }
            case SDL_SCANCODE_E: {
              if (map->areCoordsValid(player->x + 1, player->y - 1)) map->setWalkable(player->x + 1, player->y - 1, true);
              break;
            }
            default: break;
          }
          break;
        case SDL_QUIT:
          return 0;  // Exit.
      }
    } 
  }
}
