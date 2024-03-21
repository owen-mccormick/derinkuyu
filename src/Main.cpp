#include <iostream>
#include <SDL.h>
#include <libtcod.hpp>
#include <libtcod/timer.hpp>
#include "Map.hpp"
#include "Actor.hpp"
#include "Worker.hpp"
#include <queue>

int main(int argc, char* argv[]) {
  static int tickCount = 0;
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
  params.sdl_window_flags = SDL_WINDOW_MINIMIZED;
  params.vsync = true;
  params.argc = argc;  // This allows some user-control of the context.
  params.argv = argv;

  int desiredFPS = 30;
  auto timer = tcod::Timer();
  bool credits = false;

  auto context = tcod::Context(params);

  // Map, camera, and actors
  int cursorX = 25, cursorY = 20;

  Map* map = new Map(WIDTH, HEIGHT, DISPLAYWIDTH, DISPLAYHEIGHT);
  std::vector<Worker*> actors;
  Worker* worker = new Worker(map->placePlayer().first, map->placePlayer().second);
  actors.push_back(worker);
  std::priority_queue<Order> taskQueue = std::priority_queue<Order>();

  // std::pair<int, int> playerPos = map->placePlayer();
  // Actor* player = new Actor(playerPos.first, playerPos.second, "@", tcod::ColorRGB(255, 255, 255));
  // actors.push_back(player);

  while (true) {
    float deltaTime = timer.sync(desiredFPS);
    TCOD_console_clear(console.get());
    if (!credits) {
      tcod::print(console, {DISPLAYWIDTH / 50, DISPLAYHEIGHT / 3}, "Derinkuyu\nA subterranean city-building game\nBy Owen McCormick", std::nullopt, std::nullopt);
      // credits = TCOD_console_credits_render_ex(console.get(), DISPLAYWIDTH / 10, DISPLAYHEIGHT / 3 + 10, false, deltaTime * 3);
      credits = true;
    } else {
      map->render(console, cursorX, cursorY);
      for (auto actor : actors) {
        actor->render(console, cursorX, cursorY, DISPLAYWIDTH, DISPLAYHEIGHT);
      }
      if (tickCount % 4 > 1) tcod::print(console, {cursorX, DISPLAYHEIGHT / 2}, "X", std::nullopt, std::nullopt);
      // Water density display
      // tcod::print(console, {0, 0}, std::to_string(map->getWater(cursorX, cursorY)), std::nullopt, std::nullopt);
    }

    context.present(console);  // Updates the visible display.
    SDL_Event event;

    map->tick(tickCount);
    for (auto actor : actors) {
      actor->act(map, tickCount, taskQueue);
    }
    map->setMaterial(cursorX, cursorY, Material::VACUUM);
    tickCount++;
  
    // SDL_WaitEvent(nullptr);  // Optional, sleep until events are available.
    while (SDL_PollEvent(&event)) {
      context.convert_event_coordinates(event);  // Optional, converts pixel coordinates into tile coordinates.
      switch (event.type) {
        case SDL_KEYDOWN:
          switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_W: {
              if (map->areCoordsValid(cursorX, cursorY - 1)) cursorY--;
              break;
            }  
            case SDL_SCANCODE_S: {
              if (map->areCoordsValid(cursorX, cursorY + 1)) cursorY++;
              break;
            }
            case SDL_SCANCODE_A: {
              if (map->areCoordsValid(cursorX - 1, cursorY)) cursorX--;
              break;
            }
            case SDL_SCANCODE_D: {
              if (map->areCoordsValid(cursorX + 1, cursorY)) cursorX++;
              break;
            }
            case SDL_SCANCODE_SPACE: {
              // worker->order = Order(OrderType::IDLE, 0, 0, 0);
              worker->order = Order(OrderType::DIG, 0, cursorX, cursorY);
              break;
            }
            // case SDL_SCANCODE_KP_7: {
              // if (map->isWalkable(cursorX - 1, cursorY - 1) && map->areCoordsValid(cursorX - 1, cursorY - 1)) {
                // cursorX--;
                // cursorY--;
              // }
              // break;
            // }
            // case SDL_SCANCODE_KP_9: {
              // if (map->isWalkable(cursorX + 1, cursorY - 1) && map->areCoordsValid(cursorX + 1, cursorY - 1)) {
                // cursorX++;
                // cursorY--;
              // }
              // break;
            // }
            // case SDL_SCANCODE_W: {
              // if (map->areCoordsValid(cursorX, cursorY - 1)) map->setWalkable(cursorX, cursorY - 1, true);
              // break;
            // }
            // case SDL_SCANCODE_A: {
              // if (map->areCoordsValid(cursorX - 1, cursorY)) map->setWalkable(cursorX - 1, cursorY, true);
              // break;
            // }
            // case SDL_SCANCODE_S: {
              // if (map->areCoordsValid(cursorX, cursorY + 1)) map->setWalkable(cursorX, cursorY + 1, true);
              // break;
            // }
            // case SDL_SCANCODE_D: {
              // if (map->areCoordsValid(cursorX + 1, cursorY)) map->setWalkable(player-> x + 1, cursorY, true);
              // break;
            // }
            // case SDL_SCANCODE_Q: {
              // if (map->areCoordsValid(cursorX - 1, cursorY - 1)) map->setWalkable(cursorX - 1, cursorY - 1, true);
              // break;
            // }
            // case SDL_SCANCODE_E: {
              // if (map->areCoordsValid(cursorX + 1, cursorY - 1)) map->setWalkable(cursorX + 1, cursorY - 1, true);
              // break;
            // }
            default: break;
          }
          break;
        case SDL_QUIT:
          return 0; // Exit.
      }
    } 
  }
}
