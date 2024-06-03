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

// Print an indicator for a queued order
void orderOverlayPrint(tcod::Console& console, Order order, int cursorY, int displayHeight) {
  switch (order.type) {
    case (OrderType::DIG): {
      tcod::print(console, {order.interestX, order.interestY - cursorY + displayHeight / 2}, "X", TCOD_ColorRGB{255, 255, 255}, std::nullopt);
      break;
    }
    case (OrderType::BUILD): {
      tcod::print(console, {order.interestX, order.interestY - cursorY + displayHeight / 2}, std::string(1, order.interestMaterial.ch), TCOD_ColorRGB{255, 255, 255},/*std::nullopt,*/ std::nullopt);
    }
    case (OrderType::MOVE): {
      TCOD_console_put_char_ex(console.get(), order.pathX, order.pathY - cursorY + displayHeight / 2, 0x2193, TCOD_ColorRGB{255, 255, 255}, TCOD_ColorRGB{0, 0, 0});
      break;
    }
    // TODO - chop, till, plant, harvest indicators?
  }
}

// Batch designation for dig, planting, and tilling
// Used for toggling designation overlay upon associated key press
void batchDesignate(
  OrderType toDesignate, OrderType& designationType, int cursorX, int cursorY, int& designateX, int& designateY,
  std::priority_queue<Order, std::vector<Order>, compareOrders>* taskQueue
) {
  if (designationType != toDesignate) {
    designationType = toDesignate;
    designateX = cursorX;
    designateY = cursorY;
  } else {
    designationType = OrderType::IDLE;
    for (int i = designateX; designateX > cursorX ? i >= cursorX : i <= cursorX; designateX > cursorX ? i-- : i++) { // Iterate in correct direction
      for (int j = designateY; designateY > cursorY ? j >= cursorY : j <= cursorY; designateY > cursorY ? j-- : j++) {
        switch (toDesignate) {
          default: {
            // Set interest coords (but not path coords) because we don't know yet which cell adjacent to the cell of interest the miner will path to
            taskQueue->push(Order(toDesignate, 0, 0, 0, i, j));
            break;
          }
        }
      }
    }
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
  params.sdl_window_flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
  params.vsync = true;
  params.argc = argc;  // This allows some user-control of the context.
  params.argv = argv;

  int desiredFPS = 30;
  auto timer = tcod::Timer();
  bool credits = false;
  bool paused = false;

  auto context = tcod::Context(params);

  // Map, camera, and actors
  int cursorX = 25, cursorY = 20;
  int designateX = 0, designateY = 0;
  OrderType designationType = OrderType::IDLE; // Idle in this context will mean that nothing is currently being designated
  // Build menu
  int selectedBuildIndex = 0;
  Material buildMenu[7] = {Material::PLANK, Material::DOOR, Material::LADDER, Material::BED, Material::SMELTER, Material::MILLSTONE, Material::OVEN}; // Constructible blocks
  // Buy menu
  int selectedBuyIndex = 0;
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

  // For game timer
  float timeRemaining = 500;

  // std::pair<int, int> playerPos = map->placePlayer();
  // Actor* player = new Actor(playerPos.first, playerPos.second, "@", tcod::ColorRGB(255, 255, 255));
  // actors.push_back(player);

  while (true) {
    float deltaTime = timer.sync(desiredFPS);
    timeRemaining -= deltaTime;
    TCOD_console_clear(console.get());
    if (!credits) {
      timeRemaining = 500;
      // std::string title = "##           #      #               \n# # ### ###     ##  # # # # # # # # \n# # ##  #    #  # # ##  # # ### # # \n# # ### #    ## # # # # ###   # ### \n##                          ###     \n";
      // std::string title = ".-. .-. .-. .-. . . . . . . . . . . \n|  )|-  |(   |  |\\| |<  | |  |  | | \n`-' `-' ' ' `-' ' ` ' ` `-'  `  `-' \n";
      // std::string title = " HHHHHHH                  HH          HH                             \n.HH....HH                ..          .HH              HH   HH        \n.HH    .HH  HHHHH  HHHHHH HH HHHHHHH .HH  HH HH   HH ..HH HH  HH   HH\n.HH    .HH HH...HH..HH..H.HH..HH...HH.HH HH .HH  .HH  ..HHH  .HH  .HH\n.HH    .HH.HHHHHHH .HH . .HH .HH  .HH.HHHH  .HH  .HH   .HH   .HH  .HH\n.HH    HH .HH....  .HH   .HH .HH  .HH.HH.HH .HH  .HH   HH    .HH  .HH\n.HHHHHHH  ..HHHHHH.HHH   .HH HHH  .HH.HH..HH..HHHHHH  HH     ..HHHHHH\n.......    ...... ...    .. ...   .. ..  ..  ......  ..       ...... \n";
      std::string title = "######╗ #######╗######╗ ##╗###╗   ##╗##╗  ##╗##╗   ##╗##╗   ##╗##╗   ##╗\n##╔══##╗##╔════╝##╔══##╗##║####╗  ##║##║ ##╔╝##║   ##║╚##╗ ##╔╝##║   ##║\n##║  ##║#####╗  ######╔╝##║##╔##╗ ##║#####╔╝ ##║   ##║ ╚####╔╝ ##║   ##║\n##║  ##║##╔══╝  ##╔══##╗##║##║╚##╗##║##╔═##╗ ##║   ##║  ╚##╔╝  ##║   ##║\n######╔╝#######╗##║  ##║##║##║ ╚####║##║  ##╗╚######╔╝   ##║   ╚######╔╝\n╚═════╝ ╚══════╝╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝ ╚═════╝    ╚═╝    ╚═════╝ \n";
      tcod::print(console, {DISPLAYWIDTH / 50, DISPLAYHEIGHT / 5}, title, TCOD_ColorRGB{100, 85, 75}, std::nullopt);
      tcod::print(console, {DISPLAYWIDTH / 50, DISPLAYHEIGHT / 3 + 10}, "A game by Owen McCormick", TCOD_ColorRGB{155, 118, 83}, std::nullopt);
      credits = TCOD_console_credits_render_ex(console.get(), DISPLAYWIDTH / 50, DISPLAYHEIGHT / 3 + 20, false, deltaTime * 1.5);
    } else if (timeRemaining > 0) {
      map->render(console, cursorX, cursorY, tickCount);
      for (auto actor : actors) {
        // Flicker in water
        if (map->getWater(actor->getX(), actor->getY()) == 0 || tickCount % 48 >= 24)
          actor->render(console, cursorX, cursorY, DISPLAYWIDTH, DISPLAYHEIGHT, map->getWater(actor->getX(), actor->getY()));
      }
      // Inventory display
      tcod::print(console, {0, 0}, "WOOD: "  + std::to_string(inventory->wood), Material::TRUNK.fg, std::nullopt);
      tcod::print(console, {10, 0}, "SEED: " + std::to_string(inventory->cerealSeed), Material::CEREAL_SEED.fg, std::nullopt);
      tcod::print(console, {20, 0}, "GRAIN: " + std::to_string(inventory->cerealGrain), Material::CEREAL_PLANT.fg, std::nullopt);
      tcod::print(console, {31, 0}, "STONE: " + std::to_string(inventory->slate), Material::ROCK.bg, std::nullopt);
      tcod::print(console, {43, 0}, "COPPER ORE: " + std::to_string(inventory->copperOre), Material::COPPER_ORE.bg, std::nullopt);
      tcod::print(console, {60, 0}, "TIN ORE: " + std::to_string(inventory->tinOre), Material::TIN_ORE.bg, std::nullopt);
      tcod::print(console, {0, 1}, "FLOUR: " + std::to_string(inventory->flour), TCOD_ColorRGB{255, 255, 255}, std::nullopt);
      tcod::print(console, {11, 1}, "BREAD: " + std::to_string(inventory->bread), TCOD_ColorRGB{166, 42, 42}, std::nullopt);
      tcod::print(console, {22, 1}, "BRONZE: " + std::to_string(inventory->bronze), Material::COPPER_ORE.bg, std::nullopt);
      tcod::print(console, {34, 1}, "GEMS: " + std::to_string(inventory->gems), Material::GEM.bg, std::nullopt);
      tcod::print(console, {0, 2}, "MORALE: " + std::to_string(inventory->moraleBed + inventory->moraleFood), TCOD_ColorRGB{255, 255, 255}, std::nullopt);
      tcod::print(console, {12, 2}, "POINTS: " + std::to_string(inventory->points), TCOD_ColorRGB{255, 255, 255}, std::nullopt);
      tcod::print(console, {24, 2}, "REMAINING TIME: " + std::to_string((int) timeRemaining) + "s", TCOD_ColorRGB{255, 0, 0}, std::nullopt);

      // Designation markers
      std::string designateMark = "";
      switch (designationType) {
        case OrderType::DIG: {
          designateMark = "X";
          break;
        }
        case OrderType::TILL: {
          designateMark = "T";
          break;
        }
        case OrderType::PLANT: {
          designateMark = "P";
          break;
        }
        case OrderType::HARVEST: {
          designateMark = "H";
          break;
        }
        default: break;
      }
      for (int i = designateX; designateX > cursorX ? i >= cursorX : i <= cursorX; designateX > cursorX ? i-- : i++) { // Iterate in correct direction
        for (int j = designateY; designateY > cursorY ? j >= cursorY : j <= cursorY; designateY > cursorY ? j-- : j++) {
          tcod::print(console, {i, j - cursorY + DISPLAYHEIGHT / 2}, designateMark, std::nullopt, std::nullopt);
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
      tcod::print(console, {18, 45}, "BED", selectedBuildIndex == 3 ? TCOD_ColorRGB{0, 0, 0} : TCOD_ColorRGB{125, 125, 125}, selectedBuildIndex == 3 ? TCOD_ColorRGB{125, 125, 125} : TCOD_ColorRGB{0, 0, 0});
      tcod::print(console, {22, 45}, "SMELTER", selectedBuildIndex == 4 ? TCOD_ColorRGB{0, 0, 0} : TCOD_ColorRGB{125, 125, 125}, selectedBuildIndex == 4 ? TCOD_ColorRGB{125, 125, 125} : TCOD_ColorRGB{0, 0, 0});
      tcod::print(console, {30, 45}, "MILLSTONE", selectedBuildIndex == 5 ? TCOD_ColorRGB{0, 0, 0} : TCOD_ColorRGB{125, 125, 125}, selectedBuildIndex == 5 ? TCOD_ColorRGB{125, 125, 125} : TCOD_ColorRGB{0, 0, 0});
      tcod::print(console, {40, 45}, "OVEN", selectedBuildIndex == 6 ? TCOD_ColorRGB{0, 0, 0} : TCOD_ColorRGB{125, 125, 125}, selectedBuildIndex == 6 ? TCOD_ColorRGB{125, 125, 125} : TCOD_ColorRGB{0, 0, 0});

      // Update morale from food
      if (tickCount % 1600 == 0) {
        inventory->moraleFood = 0;
        for (int k = 0; k < 3; k++) {
          if (inventory->bread > 0) {
            inventory->bread--;
            inventory->moraleFood++;
          }
        }
      }

      // Print boat and trader (not "real" actors / tiles - just printed out)
      // Gives the appearance of arriving and sailing away depending on the time
      // There's probably a much more concise way to do this.
      int time = tickCount % 1600;
      bool isTraderPresent = time >= 80 && time < 520;
      if (time < 600 && time > 0) {
        // Hull
        TCOD_console_put_char_ex(console.get(), 0, 13 - cursorY + DISPLAYHEIGHT / 2, 0x2597, TCOD_ColorRGB{191, 151, 96}, TCOD_ColorRGB{156, 32, 32});
        TCOD_console_put_char_ex(console.get(), 1, 13 - cursorY + DISPLAYHEIGHT / 2, 0x2580, TCOD_ColorRGB{50, 50, 50}, TCOD_ColorRGB{156, 32, 32});
        TCOD_console_put_char_ex(console.get(), 2, 13 - cursorY + DISPLAYHEIGHT / 2, 0x2596, TCOD_ColorRGB{191, 151, 96}, TCOD_ColorRGB{156, 32, 32});
        tcod::print(console, {3, 13 - cursorY + DISPLAYHEIGHT / 2}, "#", TCOD_ColorRGB{191, 151, 96}, TCOD_ColorRGB{191, 151, 96});
        tcod::print(console, {0, 14 - cursorY + DISPLAYHEIGHT / 2}, "#", TCOD_ColorRGB{191, 151, 96}, TCOD_ColorRGB{191, 151, 96});
        TCOD_console_put_char_ex(console.get(), 1, 14 - cursorY + DISPLAYHEIGHT / 2, 0x2580, TCOD_ColorRGB{156, 32, 32}, TCOD_ColorRGB{191, 151, 96});
        tcod::print(console, {2, 14 - cursorY + DISPLAYHEIGHT / 2}, "#", TCOD_ColorRGB{191, 151, 96}, TCOD_ColorRGB{191, 151, 96});
        tcod::print(console, {1, 15 - cursorY + DISPLAYHEIGHT / 2}, "#", TCOD_ColorRGB{191, 151, 96}, TCOD_ColorRGB{191, 151, 96});
        TCOD_console_put_char_ex(console.get(), 4, 13 - cursorY + DISPLAYHEIGHT / 2, 0x2598, TCOD_ColorRGB{191, 151, 96}, TCOD_ColorRGB{map->getLight(4, 13), map->getLight(4, 13), map->getLight(4, 13)});
        TCOD_console_put_char_ex(console.get(), 3, 14 - cursorY + DISPLAYHEIGHT / 2, 0x2598, TCOD_ColorRGB{191, 151, 96}, TCOD_ColorRGB{map->getLight(3, 14), map->getLight(3, 14), map->getLight(3, 14)});
        TCOD_console_put_char_ex(console.get(), 4, 13 - cursorY + DISPLAYHEIGHT / 2, 0x2598, TCOD_ColorRGB{191, 151, 96}, TCOD_ColorRGB{map->getLight(4, 13), map->getLight(4, 13), map->getLight(4, 13)});
        TCOD_console_put_char_ex(console.get(), 0, 15 - cursorY + DISPLAYHEIGHT / 2, 0x259D, TCOD_ColorRGB{191, 151, 96}, TCOD_ColorRGB{0, 0, 125});
        TCOD_console_put_char_ex(console.get(), 2, 15 - cursorY + DISPLAYHEIGHT / 2, 0x2598, TCOD_ColorRGB{191, 151, 96}, TCOD_ColorRGB{0, 0, 125});
        // Mast and sail
        for (int j = 12; j > 8; j--) {
          TCOD_console_put_char_ex(console.get(), 1, j - cursorY + DISPLAYHEIGHT / 2, 0x2551, TCOD_ColorRGB{156, 32, 32}, TCOD_ColorRGB{map->getLight(1, j), map->getLight(1, j), map->getLight(1, j)});
          if (j < 12) {
            TCOD_console_put_char_ex(console.get(), 0, j - cursorY + DISPLAYHEIGHT / 2, 0x2593, TCOD_ColorRGB{255, 255, 255}, TCOD_ColorRGB{map->getLight(0, j - cursorY + DISPLAYHEIGHT / 2), map->getLight(0, j - cursorY + DISPLAYHEIGHT / 2),map->getLight(0, j - cursorY + DISPLAYHEIGHT / 2)});
            TCOD_console_put_char_ex(console.get(), 2, j - cursorY + DISPLAYHEIGHT / 2, 0x2593, TCOD_ColorRGB{255, 255, 255}, TCOD_ColorRGB{map->getLight(2, j - cursorY + DISPLAYHEIGHT / 2), map->getLight(2, j - cursorY + DISPLAYHEIGHT / 2),map->getLight(2, j - cursorY + DISPLAYHEIGHT / 2)});
            TCOD_console_put_char_ex(console.get(), 3, j - cursorY + DISPLAYHEIGHT / 2, 0x2593, TCOD_ColorRGB{255, 255, 255}, TCOD_ColorRGB{map->getLight(3, j - cursorY + DISPLAYHEIGHT / 2), map->getLight(3, j - cursorY + DISPLAYHEIGHT / 2),map->getLight(3, j - cursorY + DISPLAYHEIGHT / 2)});
          }
        }
        // Trader sprite
        int traderX = 5;
        if (time < 40) {
          traderX = 2;
        } else if (time >= 40 && time < 60) {
          traderX = 3;
        } else if (time >= 60 && time < 80) {
          traderX = 4;
        } else if (time >= 520 && time < 540) {
          traderX = 4;
        } else if (time >= 540 && time < 560) {
          traderX = 3;
        } else if (time >= 560 && time < 600) {
          traderX = 2;
        }
        tcod::print(console, {traderX, 12 - cursorY + DISPLAYHEIGHT / 2}, "&", TCOD_ColorRGB{255, 255, 0}, std::nullopt);
      } else if ((time >= 600 && time < 800) || (time > 1400)) {
        TCOD_console_put_char_ex(console.get(), 0, 14 - cursorY + DISPLAYHEIGHT / 2, 0x259D, TCOD_ColorRGB{191, 151, 96}, TCOD_ColorRGB{map->getLight(0, 14), map->getLight(0, 14),map->getLight(0, 14)});
        TCOD_console_put_char_ex(console.get(), 1, 14 - cursorY + DISPLAYHEIGHT / 2, 0x2580, TCOD_ColorRGB{191, 151, 96}, TCOD_ColorRGB{191, 151, 96});
        TCOD_console_put_char_ex(console.get(), 2, 14 - cursorY + DISPLAYHEIGHT / 2, 0x2598, TCOD_ColorRGB{191, 151, 96}, TCOD_ColorRGB{map->getLight(2, 14), map->getLight(2, 14), map->getLight(2, 14)});
        TCOD_console_put_char_ex(console.get(), 1, 13 - cursorY + DISPLAYHEIGHT / 2, '|', TCOD_ColorRGB{156, 32, 32}, TCOD_ColorRGB{255, 255, 255});
        TCOD_console_put_char_ex(console.get(), 2, 13 - cursorY + DISPLAYHEIGHT / 2, 0x2597, TCOD_ColorRGB{255, 255, 0}, TCOD_ColorRGB{map->getLight(2, 13), map->getLight(2, 13), map->getLight(2, 13)});
      }

      // Actions when hovering over trader
      if (cursorX == 5 && cursorY == 12 && isTraderPresent) {
        if (inventory->moraleFood + inventory->moraleBed >= 6) {
          tcod::print(console, {0, 43}, "BUY WOOD: -2 PTS", selectedBuyIndex == 0 ? TCOD_ColorRGB{0, 0, 0} : TCOD_ColorRGB{125, 125, 125}, selectedBuyIndex == 0 ? TCOD_ColorRGB{125, 125, 125} : TCOD_ColorRGB{0, 0, 0});
          tcod::print(console, {17, 43}, "SELL BRONZE: +6 PTS", selectedBuyIndex == 1 ? TCOD_ColorRGB{0, 0, 0} : TCOD_ColorRGB{125, 125, 125}, selectedBuyIndex == 1 ? TCOD_ColorRGB{125, 125, 125} : TCOD_ColorRGB{0, 0, 0});
          tcod::print(console, {37, 43}, "SELL FLOUR: +2 PTS", selectedBuyIndex == 2 ? TCOD_ColorRGB{0, 0, 0} : TCOD_ColorRGB{125, 125, 125}, selectedBuyIndex == 2 ? TCOD_ColorRGB{125, 125, 125} : TCOD_ColorRGB{0, 0, 0});
          tcod::print(console, {56, 43}, "SELL GEMS: +10 PTS", selectedBuyIndex == 3 ? TCOD_ColorRGB{0, 0, 0} : TCOD_ColorRGB{125, 125, 125}, selectedBuyIndex == 3 ? TCOD_ColorRGB{125, 125, 125} : TCOD_ColorRGB{0, 0, 0});
        } else {
          tcod::print(console, {0, 43}, "TRADING NOT AVAILABLE - MORALE MUST BE 6 OR GREATER!", TCOD_ColorRGB{0, 0, 0}, TCOD_ColorRGB{125, 125, 125});
        }
      }

      // Print scheduled task indicators
      // We can't iterate through the queue, so we empty it to a vector and restore it afterwards
      std::vector<Order> orders;
      while (!taskQueue->empty()) {
        orders.push_back(taskQueue->top());
        taskQueue->pop();
      }
      for (auto order : orders) {
        orderOverlayPrint(console, order, cursorY, DISPLAYHEIGHT);
        taskQueue->push(order);
      }
      for (auto actor : actors) {
        if (actor->order.type != OrderType::DIG) {
          orderOverlayPrint(console, actor->order, cursorY, DISPLAYHEIGHT);
        }
      }

      if (!paused) {
        map->tick(tickCount, false);
        for (auto actor : actors) {
          actor->act(tickCount, isTraderPresent);
        }
      }

      // Cursor
      if (tickCount % 36 >= 0) tcod::print(console, {cursorX, DISPLAYHEIGHT / 2}, "X", TCOD_ColorRGB{255, 255, 0}, std::nullopt);
    } else {
      // Out of time
      for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
          if (!map->getDisintegrate(i, j)) map->setDisintegrate(i, j, true);
        }
      }
      map->tick(tickCount, true);
      map->render(console, cursorX, cursorY, tickCount);
      if (timeRemaining < -3) {
        tcod::print(console, {DISPLAYWIDTH / 2 - 8, DISPLAYHEIGHT / 2 - 1}, "THE END...", TCOD_ColorRGB{100, 85, 75}, std::nullopt);
        tcod::print(console, {DISPLAYWIDTH / 2 - 8, DISPLAYHEIGHT / 2 + 1}, "SCORE: " + std::to_string(inventory->points), TCOD_ColorRGB{100, 85, 75}, std::nullopt);
      }
    }

    context.present(console);  // Updates the visible display.
    SDL_Event event;

    if (!paused) tickCount++;
  
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
              if (!credits) {
                credits = true; // Trigger intro skip
              } else {
                taskQueue->push(Order(OrderType::MOVE, 0, cursorX, cursorY, 0, 0));
              }
              break;
            }
            case SDL_SCANCODE_RETURN: {
              paused = !paused;
              break;
            }
            case SDL_SCANCODE_E: {
              batchDesignate(OrderType::DIG, designationType, cursorX, cursorY, designateX, designateY, taskQueue);
              break;
            }
            case SDL_SCANCODE_I: {
              batchDesignate(OrderType::TILL, designationType, cursorX, cursorY, designateX, designateY, taskQueue);
              break;
            }
            case SDL_SCANCODE_O: {
              batchDesignate(OrderType::PLANT, designationType, cursorX, cursorY, designateX, designateY, taskQueue);
              break;
            }
            case SDL_SCANCODE_P: {
              batchDesignate(OrderType::HARVEST, designationType, cursorX, cursorY, designateX, designateY, taskQueue);
            }
            case SDL_SCANCODE_Q: {
              taskQueue->push(Order(OrderType::CHOP, 0, 0, 0, cursorX, cursorY));
              break;
            }
            case SDL_SCANCODE_R: {
              if (map->getMaterial(cursorX, cursorY).id == Material::SMELTER.id) {
                taskQueue->push(Order(OrderType::FABRICATE, 0, 0, 0, cursorX, cursorY, FabricateType::SMELT));
              } else if (map->getMaterial(cursorX, cursorY).id == Material::MILLSTONE.id) {
                taskQueue->push(Order(OrderType::FABRICATE, 0, 0, 0, cursorX, cursorY, FabricateType::MILL));
              } else if (map->getMaterial(cursorX, cursorY).id == Material::OVEN.id) {
                taskQueue->push(Order(OrderType::FABRICATE, 0, 0, 0, cursorX, cursorY, FabricateType::BAKE));
              }
              break;
            }
            // Z, X, and C control both the build and buy / sell menus depending on whether or not the cursor is hovering over the trader
            // Build designations and build menu left and right
            case SDL_SCANCODE_Z: {
              int time = tickCount % 1600;
              if (cursorX == 5 && cursorY == 12 && time >= 80 && time < 520 && selectedBuyIndex > 0) {
                selectedBuyIndex--;
              } else if (selectedBuildIndex > 0) selectedBuildIndex--;
              break;
            }
            case SDL_SCANCODE_X: {
              int time = tickCount % 1600;
              if (cursorX == 5 && cursorY == 12 && time >= 80 && time < 520) {
                if (inventory->moraleBed + inventory->moraleFood >= 6) {
                  if (selectedBuyIndex == 0 && inventory->points >= 2) {
                    taskQueue->push(Order(OrderType::TRADE, 1, 0, 0, 0, 0, TradeType::BUY_WOOD));
                  } else if (selectedBuyIndex == 1 && inventory->bronze > 0) {
                    taskQueue->push(Order(OrderType::TRADE, 1, 0, 0, 0, 0, TradeType::SELL_BRONZE));
                  } else if (selectedBuyIndex == 2 && inventory->flour > 0) {
                    taskQueue->push(Order(OrderType::TRADE, 1, 0, 0, 0, 0, TradeType::SELL_FLOUR));
                  } else if (selectedBuyIndex == 3 && inventory->gems > 0) {
                    taskQueue->push(Order(OrderType::TRADE, 1, 0, 0, 0, 0, TradeType::SELL_GEMS));
                  }
                }
              } else {
                if (selectedBuildIndex == 0) {
                  taskQueue->push(Order(OrderType::BUILD, 0, 0, 0, cursorX, cursorY, Material::PLANK));
                } else if (selectedBuildIndex == 1) {
                  taskQueue->push(Order(OrderType::BUILD, 0, 0, 0, cursorX, cursorY, Material::DOOR));
                } else if (selectedBuildIndex == 2) {
                  taskQueue->push(Order(OrderType::BUILD, 0, 0, 0, cursorX, cursorY, Material::LADDER));
                } else if (selectedBuildIndex == 3) {
                  taskQueue->push(Order(OrderType::BUILD, 0, 0, 0, cursorX, cursorY, Material::BED));
                } else if (selectedBuildIndex == 4) {
                  taskQueue->push(Order(OrderType::BUILD, 0, 0, 0, cursorX, cursorY, Material::SMELTER));
                } else if (selectedBuildIndex == 5) {
                  taskQueue->push(Order(OrderType::BUILD, 0, 0, 0, cursorX, cursorY, Material::MILLSTONE));
                } else if (selectedBuildIndex == 6) {
                  taskQueue->push(Order(OrderType::BUILD, 0, 0, 0, cursorX, cursorY, Material::OVEN));
                }
              }
              break;
            }
            case SDL_SCANCODE_C: {
              int time = tickCount % 1600;
              if (cursorX == 5 && cursorY == 12 && time >= 80 && time < 520 && selectedBuyIndex < 3) {
                selectedBuyIndex++;
              } else if (selectedBuildIndex < 6) selectedBuildIndex++;
              break;
            }
            case SDL_SCANCODE_BACKSPACE: {
              for (auto actor : actors) {
                actor->order = Order(OrderType::IDLE, 0, 0, 0, 0, 0);
              }
              while (!taskQueue->empty()) taskQueue->pop();
              break;
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
          delete map;
          delete taskQueue;
          delete inventory;
          delete worker0;
          delete worker1;
          delete worker2;
          return 0; // Exit.
      }
    } 
  }
}
