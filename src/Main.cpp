#include <SDL.h>
#include <libtcod.hpp>
#include <libtcod/timer.hpp>
#include "Map.hpp"
#include "Actor.hpp"
#include "Worker.hpp"
#include "Inventory.hpp"
#include <queue>
#include <vector>
#include <iostream>

// Print an indicator for a queued order - not functional and not used yet
void orderOverlayPrint(tcod::Console& console, Order order) {
  // TODO - translate from coordinate to displayed tile space on screen
  switch (order.type) {
    case (OrderType::DIG): {
      tcod::print(console, {order.interestX, order.interestY}, "X", TCOD_ColorRGB{255, 255, 255}, std::nullopt);
      break;
    }
    case (OrderType::BUILD): {
      tcod::print(console, {order.interestX, order.interestY}, std::string(1, order.interestMaterial.ch), TCOD_ColorRGB{255, 255, 255}, std::nullopt);
    }
    // TODO - move and chop indicators?
  }
}

int main(int argc, char* argv[]) {
  static int tickCount = 0;
  static constexpr int WIDTH = 75;
  static constexpr int HEIGHT = 86;
  static constexpr int DISPLAYWIDTH = 75;
  static constexpr int DISPLAYHEIGHT = 46;
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
  int digDesignateX = 0, digDesignateY = 0;
  bool designatingDig = false;
  int selectedBuildIndex = 0;
  Material buildMenu[3] = {Material::PLANK, Material::DOOR, Material::LADDER}; // Constructible blocks

  std::vector<Worker*> actors;
  Inventory* inventory = new Inventory(); // May want to switch away from the simple global inventory system at some point
  Map* map = new Map(inventory, WIDTH, HEIGHT, DISPLAYWIDTH, DISPLAYHEIGHT);
  std::priority_queue<Order, std::vector<Order>, compareOrders>* taskQueue = new std::priority_queue<Order, std::vector<Order>, compareOrders>();
  Worker* worker0 = new Worker(map, taskQueue, inventory, map->placePlayer().first, map->placePlayer().second);
  Worker* worker1 = new Worker(map, taskQueue, inventory, map->placePlayer().first, map->placePlayer().second);
  Worker* worker2 = new Worker(map, taskQueue, inventory, map->placePlayer().first, map->placePlayer().second);
  actors.push_back(worker0);
  actors.push_back(worker1);
  actors.push_back(worker2);

  // std::pair<int, int> playerPos = map->placePlayer();
  // Actor* player = new Actor(playerPos.first, playerPos.second, "@", tcod::ColorRGB(255, 255, 255));
  // actors.push_back(player);

  while (true) {
    float deltaTime = timer.sync(desiredFPS);
    TCOD_console_clear(console.get());
    if (!credits) {
      // std::string title = "##           #      #               \n# # ### ###     ##  # # # # # # # # \n# # ##  #    #  # # ##  # # ### # # \n# # ### #    ## # # # # ###   # ### \n##                          ###     \n";
      // std::string title = ".-. .-. .-. .-. . . . . . . . . . . \n|  )|-  |(   |  |\\| |<  | |  |  | | \n`-' `-' ' ' `-' ' ` ' ` `-'  `  `-' \n";
      // std::string title = " HHHHHHH                  HH          HH                             \n.HH....HH                ..          .HH              HH   HH        \n.HH    .HH  HHHHH  HHHHHH HH HHHHHHH .HH  HH HH   HH ..HH HH  HH   HH\n.HH    .HH HH...HH..HH..H.HH..HH...HH.HH HH .HH  .HH  ..HHH  .HH  .HH\n.HH    .HH.HHHHHHH .HH . .HH .HH  .HH.HHHH  .HH  .HH   .HH   .HH  .HH\n.HH    HH .HH....  .HH   .HH .HH  .HH.HH.HH .HH  .HH   HH    .HH  .HH\n.HHHHHHH  ..HHHHHH.HHH   .HH HHH  .HH.HH..HH..HHHHHH  HH     ..HHHHHH\n.......    ...... ...    .. ...   .. ..  ..  ......  ..       ...... \n";
      std::string title = "######╗ #######╗######╗ ##╗###╗   ##╗##╗  ##╗##╗   ##╗##╗   ##╗##╗   ##╗\n##╔══##╗##╔════╝##╔══##╗##║####╗  ##║##║ ##╔╝##║   ##║╚##╗ ##╔╝##║   ##║\n##║  ##║#####╗  ######╔╝##║##╔##╗ ##║#####╔╝ ##║   ##║ ╚####╔╝ ##║   ##║\n##║  ##║##╔══╝  ##╔══##╗##║##║╚##╗##║##╔═##╗ ##║   ##║  ╚##╔╝  ##║   ##║\n######╔╝#######╗##║  ##║##║##║ ╚####║##║  ##╗╚######╔╝   ##║   ╚######╔╝\n╚═════╝ ╚══════╝╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝ ╚═════╝    ╚═╝    ╚═════╝ \n";
      tcod::print(console, {DISPLAYWIDTH / 50, DISPLAYHEIGHT / 5}, title, TCOD_ColorRGB{100, 85, 75}, std::nullopt);
      tcod::print(console, {DISPLAYWIDTH / 50, DISPLAYHEIGHT / 3 + 10}, "A sandbox city-building game by Owen McCormick", TCOD_ColorRGB{155, 118, 83}, std::nullopt);
      credits = TCOD_console_credits_render_ex(console.get(), DISPLAYWIDTH / 50, DISPLAYHEIGHT / 3 + 20, false, deltaTime * 1.5);
    } else {
      map->render(console, cursorX, cursorY, tickCount);
      for (auto actor : actors) {
        // Flicker in water
        if (map->getWater(actor->getX(), actor->getY()) == 0 || tickCount % 48 >= 24)
          actor->render(console, cursorX, cursorY, DISPLAYWIDTH, DISPLAYHEIGHT, map->getWater(actor->getX(), actor->getY()));
      }
      if (tickCount % 36 >= 0) tcod::print(console, {cursorX, DISPLAYHEIGHT / 2}, "X", TCOD_ColorRGB{255, 255, 0}, std::nullopt);
      tcod::print(console, {0, 0}, "WOOD: "  + std::to_string(inventory->wood), Material::TRUNK.fg, std::nullopt); // Inventory display
      map->tick(tickCount);
      for (auto actor : actors) {
        actor->act(tickCount);
      }
      // Dig designation marker
      if (designatingDig) {
        for (int i = digDesignateX; digDesignateX > cursorX ? i >= cursorX : i <= cursorX; digDesignateX > cursorX ? i-- : i++) { // Iterate in correct direction
          for (int j = digDesignateY; digDesignateY > cursorY ? j >= cursorY : j <= cursorY; digDesignateY > cursorY ? j-- : j++) {
            tcod::print(console, {i, j - cursorY + DISPLAYHEIGHT / 2}, "X", std::nullopt, std::nullopt);
          }
        }
      }

      // Print build menu
      // TODO - figure out why this doesn't work
      // int spacing = 0;
      // sizeof is for finding the array length
      // for (int i = 0; i < sizeof(buildMenu) / sizeof(*buildMenu); i++) {
        // std::cout << "Build menu display for: " << buildMenu[i].name << std::endl;
        // tcod::print(console, {spacing, 1}, buildMenu[i].name, std::nullopt, std::nullopt);//selectedBuildIndex == i ? TCOD_ColorRGB{125, 125, 125} : TCOD_ColorRGB{0, 0, 0});
        // spacing += buildMenu[i].name.length() + 1;
      // }
      tcod::print(console, {0, 45}, "PLANK", selectedBuildIndex == 0 ? TCOD_ColorRGB{0, 0, 0} : TCOD_ColorRGB{125, 125, 125}, selectedBuildIndex == 0 ? TCOD_ColorRGB{125, 125, 125} : TCOD_ColorRGB{0, 0, 0});
      tcod::print(console, {6, 45}, "DOOR", selectedBuildIndex == 1 ? TCOD_ColorRGB{0, 0, 0} : TCOD_ColorRGB{125, 125, 125}, selectedBuildIndex == 1 ? TCOD_ColorRGB{125, 125, 125} : TCOD_ColorRGB{0, 0, 0});
      tcod::print(console, {11, 45}, "LADDER", selectedBuildIndex == 2 ? TCOD_ColorRGB{0, 0, 0} : TCOD_ColorRGB{125, 125, 125}, selectedBuildIndex == 2 ? TCOD_ColorRGB{125, 125, 125} : TCOD_ColorRGB{0, 0, 0});

      // Print scheduled task indicators
      // TODO - figure out how to iterate through taskQueue
      // This is also incompatible with the nice-looking block break animation
    }

    context.present(console);  // Updates the visible display.
    SDL_Event event;

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
              taskQueue->push(Order(OrderType::MOVE, 0, cursorX, cursorY, 0, 0));
              credits = true; // Trigger intro skip
              break;
            }
            case SDL_SCANCODE_E: {
              if (!designatingDig) {
                designatingDig = true;
                digDesignateX = cursorX;
                digDesignateY = cursorY;
              } else {
                designatingDig = false;
                for (int i = digDesignateX; digDesignateX > cursorX ? i >= cursorX : i <= cursorX; digDesignateX > cursorX ? i-- : i++) { // Iterate in correct direction
                  for (int j = digDesignateY; digDesignateY > cursorY ? j >= cursorY : j <= cursorY; digDesignateY > cursorY ? j-- : j++) {
                    // Set interest coords (but not path coords) because we don't know yet which cell adjacent to the cell of interest the miner will path to
                    taskQueue->push(Order(OrderType::DIG, 0, 0, 0, i, j));
                  }
                }
              }
              break;
            }
            case SDL_SCANCODE_Q: {
              taskQueue->push(Order(OrderType::CHOP, 0, 0, 0, cursorX, cursorY));
              break;
            }
            // Build designations and build menu left and right
            case SDL_SCANCODE_Z: {
              if (selectedBuildIndex > 0) selectedBuildIndex--;
              break;
            }
            case SDL_SCANCODE_X: {
              if (inventory->wood > 0) {
                if (selectedBuildIndex == 0) {
                  taskQueue->push(Order(OrderType::BUILD, 0, 0, 0, cursorX, cursorY, Material::PLANK));
                  // taskQueue->push(Order(OrderType::MOVE, 0, cursorX, cursorY, 0, 0));
                } else if (selectedBuildIndex == 1) {
                  taskQueue->push(Order(OrderType::BUILD, 0, 0, 0, cursorX, cursorY, Material::DOOR));
                  // taskQueue->push(Order(OrderType::MOVE, 0, cursorX, cursorY, 0, 0));
                } else if (selectedBuildIndex == 2) {
                  taskQueue->push(Order(OrderType::BUILD, 0, 0, 0, cursorX, cursorY, Material::LADDER));
                  // taskQueue->push(Order(OrderType::MOVE, 0, cursorX, cursorY, 0, 0));
                }
                inventory->wood--;
              }
              break;
            }
            case SDL_SCANCODE_C: {
              if (selectedBuildIndex < 2) selectedBuildIndex++;
              break;
            }
            case SDL_SCANCODE_P: {
              std::cout << "Task queue length:" << std::to_string(taskQueue->size()) << std::endl;
              break;
            }
            case SDL_SCANCODE_BACKSPACE: {
              for (auto actor : actors) {
                actor->order = Order(OrderType::IDLE, 0, 0, 0, 0, 0);
              }
              while (!taskQueue->empty()) taskQueue->pop();
            }
            // case SDL_SCANCODE_BACKSPACE: {
              // map->setMaterial(cursorX, cursorY, Material::VACUUM);
              // break;
            // }
            // case SDL_SCANCODE_B: {
              // map->setMaterial(cursorX, cursorY, Material::DOOR);
              // break;
            // }
            // case SDL_SCANCODE_V: {
              // map->setMaterial(cursorX, cursorY, Material::LADDER);
              // break;
            // }
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
          // delete map;
          // delete taskQueue;
          // delete worker0;
          // delete worker1;
          // delete worker2;
          return 0; // Exit.
      }
    } 
  }
}
