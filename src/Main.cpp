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
  static constexpr int WIDTH = 75;
  static constexpr int HEIGHT = 86;
  static constexpr int DISPLAYWIDTH = 75;
  static constexpr int DISPLAYHEIGHT = 45;
  auto console = tcod::Console{DISPLAYWIDTH, DISPLAYHEIGHT};
  auto tileset = tcod::load_tilesheet("data/fonts/dejavu8x8_gs_tc.png", {32, 8}, tcod::CHARMAP_TCOD);
  auto params = TCOD_ContextParams{};
  params.tcod_version = TCOD_COMPILEDVERSION;  // This is required.
  params.console = console.get();  // Derive the window size from the console size.
  params.tileset = tileset.get();
  params.window_title = "Derinkuyu";
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

  std::vector<Worker*> actors;
  Map* map = new Map(WIDTH, HEIGHT, DISPLAYWIDTH, DISPLAYHEIGHT);
  Worker* worker0 = new Worker(map->placePlayer().first, map->placePlayer().second);
  Worker* worker1 = new Worker(map->placePlayer().first, map->placePlayer().second);
  Worker* worker2 = new Worker(map->placePlayer().first, map->placePlayer().second);
  actors.push_back(worker0);
  actors.push_back(worker1);
  actors.push_back(worker2);
  std::priority_queue<Order> taskQueue = std::priority_queue<Order>();

  // std::pair<int, int> playerPos = map->placePlayer();
  // Actor* player = new Actor(playerPos.first, playerPos.second, "@", tcod::ColorRGB(255, 255, 255));
  // actors.push_back(player);

  while (true) {
    float deltaTime = timer.sync(desiredFPS);
    TCOD_console_clear(console.get());
    if (!credits) {
      // std::string title = "##           #      #               \n# # ### ###     ##  # # # # # # # # \n# # ##  #    #  # # ##  # # ### # # \n# # ### #    ## # # # # ###   # ### \n##                          ###     \n";
      // std::string title = ".-. .-. .-. .-. . . . . . . . . . . \n|  )|-  |(   |  |\\| |<  | |  |  | | \n`-' `-' ' ' `-' ' ` ' ` `-'  `  `-' \n";
      std::string title = " HHHHHHH                  HH          HH                             \n.HH....HH                ..          .HH              HH   HH        \n.HH    .HH  HHHHH  HHHHHH HH HHHHHHH .HH  HH HH   HH ..HH HH  HH   HH\n.HH    .HH HH...HH..HH..H.HH..HH...HH.HH HH .HH  .HH  ..HHH  .HH  .HH\n.HH    .HH.HHHHHHH .HH . .HH .HH  .HH.HHHH  .HH  .HH   .HH   .HH  .HH\n.HH    HH .HH....  .HH   .HH .HH  .HH.HH.HH .HH  .HH   HH    .HH  .HH\n.HHHHHHH  ..HHHHHH.HHH   .HH HHH  .HH.HH..HH..HHHHHH  HH     ..HHHHHH\n.......    ...... ...    .. ...   .. ..  ..  ......  ..       ...... \n";
      tcod::print(console, {DISPLAYWIDTH / 50, DISPLAYHEIGHT / 5}, title, TCOD_ColorRGB{100, 85, 75}, std::nullopt);
      tcod::print(console, {DISPLAYWIDTH / 50, DISPLAYHEIGHT / 3 + 10}, "A subterranean city-building game by Owen McCormick", TCOD_ColorRGB{155, 118, 83}, std::nullopt);
      credits = TCOD_console_credits_render_ex(console.get(), DISPLAYWIDTH / 50, DISPLAYHEIGHT / 3 + 20, false, deltaTime * 2);
    } else {
      map->render(console, cursorX, cursorY, tickCount);
      for (auto actor : actors) {
        // Flicker in water
        if (map->getWater(actor->getX(), actor->getY()) == 0 || tickCount % 48 >= 24)
          actor->render(console, cursorX, cursorY, DISPLAYWIDTH, DISPLAYHEIGHT, map->getWater(actor->getX(), actor->getY()));
      }
      if (tickCount % 36 >= 0) tcod::print(console, {cursorX, DISPLAYHEIGHT / 2}, "X", TCOD_ColorRGB{255, 255, 0}, std::nullopt);
      // Water density display
      // tcod::print(console, {0, 0}, std::to_string(map->getWater(cursorX, cursorY)), std::nullopt, std::nullopt);
      tcod::print(console, {0, 0}, std::to_string(cursorX) + ", " + std::to_string(cursorY), std::nullopt, std::nullopt);
      map->tick(tickCount);
      for (auto actor : actors) {
        actor->act(map, tickCount, taskQueue);
      }
    }

    context.present(console);  // Updates the visible display.
    SDL_Event event;

    // map->setMaterial(cursorX, cursorY, Material::VACUUM);
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
              worker0->order = Order(OrderType::MOVE, 0, cursorX, cursorY);
              worker1->order = Order(OrderType::MOVE, 0, cursorX, cursorY);
              worker2->order = Order(OrderType::MOVE, 0, cursorX, cursorY);
              credits = true;
              break;
            }
            case SDL_SCANCODE_BACKSPACE: {
              map->setMaterial(cursorX, cursorY, Material::VACUUM);
              break;
            }
            case SDL_SCANCODE_B: {
              map->setMaterial(cursorX, cursorY, Material::LADDER);
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
