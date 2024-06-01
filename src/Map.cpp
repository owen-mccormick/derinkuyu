#include <libtcod.hpp>
#include "Map.hpp"
#include <algorithm>
#include <iostream>
#include "math.h"

// Materials (maybe should be done in .hpp)
const Material Material::VACUUM = Material("VACUUM", true, false, 0x00, 0, TCOD_ColorRGB{0, 0, 0}, TCOD_ColorRGB{255, 255, 255});
const Material Material::ROCK = Material("ROCK", false, false, 0x2593, 0x2592, 0x2591, 1, TCOD_ColorRGB{100, 85, 75}, TCOD_ColorRGB{255, 255, 255});
const Material Material::TRUNK = Material("TRUNK", true, false, 0x2551, 2, TCOD_ColorRGB{166, 42, 42}, TCOD_ColorRGB{255, 255, 255}); 
const Material Material::LEAVES = Material("LEAVES", true, false, '#', 3, TCOD_ColorRGB{0, 255, 0}, TCOD_ColorRGB{255, 255, 255});
const Material Material::DIRT = Material("DIRT", false, false, 0x2593, 0x2592, 0x2591, 4, TCOD_ColorRGB{155, 118, 83}, TCOD_ColorRGB{255, 255, 255});
const Material Material::GRASS = Material("GRASS", true, false, '_', 5, TCOD_ColorRGB{0, 255, 0}, TCOD_ColorRGB{255, 255, 255});
const Material Material::LADDER = Material("LADDER", true, true, 'H', 6, TCOD_ColorRGB{166, 42, 42}, TCOD_ColorRGB{255, 255, 255});
const Material Material::WHEEL = Material("WHEEL", true, false, 0x25C9, 7, TCOD_ColorRGB{156, 32, 32}, TCOD_ColorRGB{255, 255, 255});
const Material Material::PLANK = Material("PLANK", false, false, 0x2550, 7, TCOD_ColorRGB{156, 32, 32}, TCOD_ColorRGB{255, 255, 255});
const Material Material::CANOPY = Material("CANOPY", true, false, 0x2593, 7, TCOD_ColorRGB{255, 255, 255}, TCOD_ColorRGB{255, 255, 255});
const Material Material::DOOR = Material("DOOR", true, false, true, '[', 8, TCOD_ColorRGB{166, 42, 42}, TCOD_ColorRGB{255, 255, 255});
const Material Material::BED = Material("BED", true, false, 'm', 9, TCOD_ColorRGB{166, 42, 42}, TCOD_ColorRGB{255, 255, 255});
const Material Material::FARM_PLOT = Material("FARM PLOT", false, false, 0x2550, 10, TCOD_ColorRGB{155, 118, 83}, TCOD_ColorRGB{255, 255, 255});
const Material Material::CEREAL_SEED = Material("CEREAL SEED", true, false, '.', 11, TCOD_ColorRGB{255, 255, 0}, TCOD_ColorRGB{255, 255, 255});
const Material Material::CEREAL_PLANT = Material("CEREAL PLANT", true, false, 0x2551, 12, TCOD_ColorRGB{255, 255, 0}, TCOD_ColorRGB{255, 255, 255});
const Material Material::COPPER_ORE = Material("COPPER ORE", false, false, 0x2593, 0x2592, 0x2591, 13, TCOD_ColorRGB{100, 85, 75}, TCOD_ColorRGB{125, 75, 0});
const Material Material::TIN_ORE = Material("TIN ORE", false, false, 0x2593, 0x2592, 0x2591, 14, TCOD_ColorRGB{100, 85, 75}, TCOD_ColorRGB{180, 180, 180});
const Material Material::SMELTER = Material("SMELTER", true, false, 'U', 15, TCOD_ColorRGB{100, 85, 75}, TCOD_ColorRGB{255, 255, 255});
const Material Material::MILLSTONE = Material("MILLSTONE", true, false, 'O', 16, TCOD_ColorRGB{100, 85, 75}, TCOD_ColorRGB{255, 255, 255});

Map::Map(Inventory* inventory, int width, int height, int displayWidth, int displayHeight) : width(width),
    height(height), displayWidth(displayWidth), displayHeight(displayHeight), inventory(inventory) {

  tiles = new Tile[width * height];
  TCODRandom* rng = TCODRandom::getInstance();

  // Sunlight
  for (int i = 0; i < width; i++) {
    tiles[i + width].emitsLight = true;
  }

  // Surface layer
  for (int i = 0; i < width; i++) {
    setWalkable(i, 14, false);
  }

  // Use Bresehnam line tools to draw tree trunks
  for (int k = 0; k < 15; k++) {
    TCOD_bresenham_data_t data;
    int x = rng->getInt(0, width), y = 13;
    if (x > 8) { // Leave far part of map clear - space for moon in the sky
      TCOD_line_init_mt(x, y, x + rng->getInt(-1, 1), 5, &data);
      do {
        setMaterial(x, y, Material::TRUNK);
      } while (!TCOD_line_step_mt(&x, &y, &data));
    }
  }
  // Leaves
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < 10; j++) {
      for (int xOff = -1; xOff < 2; xOff++) {
        for (int yOff = -1; yOff < 2; yOff++) {
          if (areCoordsValid(i + xOff, j + yOff) && getMaterial(i + xOff, j + yOff).id == Material::TRUNK.id && getMaterial(i, j).id != Material::TRUNK.id) {
            setMaterial(i, j, Material::LEAVES);
          }
        }
      }
    }
  }

  // Grass
  for (int k = 0; k < 6; k++) {
    setMaterial(rng->getInt(0, width), 13, Material::GRASS);
  }

  // Place wagon
  wagonX = rng->getInt(0, width - 6);
  // setMaterial(wagonX, 13, Material::WHEEL);
  // setMaterial(wagonX + 1, 13, Material::WHEEL);
  // setMaterial(wagonX + 3, 13, Material::WHEEL);
  // setMaterial(wagonX + 4, 13, Material::WHEEL);
  // for (int i = wagonX; i < wagonX + 5; i++) {
    // setMaterial(i, 12, Material::PLANK);
    // setMaterial(i, 11, Material::CANOPY);
  // }

  // Cellular automata cave creation based on https://www.roguebasin.com/index.php?title=Cellular_Automata_Method_for_Generating_Random_Cave-Like_Levels
  for (int i = 0; i < width; i++) {
    for (int j = 15; j < height; j++) {
      if (rng->getInt(0, 100) < 65) {
        setWalkable(i, j, false);
      }
    }
  }

  for (int k = 0; k < 5; k++) {
    for (int i = 0; i < width - 1; i++) {
      for (int j = 15; j < height - 1; j++) {
        int neighborWalls = 0;
        for (int xOff = -1; xOff < 2; xOff++) {
          for (int yOff = -1; yOff < 2; yOff++) {
            if (isWalkable(xOff + i, yOff + j)) {
              neighborWalls++;
            }
          }
        }
        if (neighborWalls > 4) {
          setWalkable(i, j, true);
        } else if (neighborWalls < 4) {
          setWalkable(i, j, false);
        }
      }
    }
  }

  // Ore veins: paint random lines on the bottom third of the map
  for (int k = 0; k <= 15; k++) {
    TCOD_bresenham_data_t data;
    int x = rng->getInt(0, width), y = rng->getInt(height / 2, height);
    TCOD_line_init_mt(x, y, rng->getInt(0, width), rng->getInt(height / 2, height), &data);
    do {
      if (getMaterial(x, y).id == Material::ROCK.id) setMaterial(x, y, k < 10 ? Material::COPPER_ORE : Material::TIN_ORE);
    } while (!TCOD_line_step_mt(&x, &y, &data));
  }

  // Designate a handful of non-adjacent sky tiles as stars that don't dim entirely at night
  // for (int k = 0; k < 10; k++) {
    // int x = rng->getInt(0, width), y = rng->getInt(0, 14);
    // bool adjacent = false;
    // for (int i = -1; i <= 1; i++) {
      // for (int j = -1; j <= 1; j++) {
        // if (getTile(x + i, y + j)->star) adjacent = true;
      // }
    // }
    // if (!adjacent) getTile(x, y)->star = true;
  // }

  // Moon?
  getTile(4, 3)->star = true;
  getTile(4, 2)->star = true;
  getTile(4, 4)->star = true;
  getTile(5, 1)->star = true;
  getTile(5, 2)->star = true;
  getTile(5, 3)->star = true;
  getTile(5, 4)->star = true;
  getTile(5, 5)->star = true;
  getTile(6, 1)->star = true;
  getTile(6, 5)->star = true;

}

Tile* Map::getTile(int x, int y) {
  return &tiles[x + y * width];
}

std::pair<int, int> Map::placePlayer() {
  TCODRandom* rng = TCODRandom::getInstance();
  int x, y;
  do {
    x = wagonX + rng->getInt(-5, 5);
    y = 13;
  } while (!isActorWalkable(x, y));
  return {x, y};
}

Map::~Map() {
  delete [] tiles;
}

// Used in terrain generation
void Map::setWalkable(int x, int y, bool walk) {
  TCODRandom* rng = TCODRandom::getInstance();
  tiles[x + y * width].material = walk ? Material::VACUUM : rng->getInt(0, 100) > 5 * y - 5 * 14 ? Material::DIRT : Material::ROCK;
}

bool Map::isWalkable(int x, int y) {
  return areCoordsValid(x, y) && tiles[x + y * width].material.passable;
}

bool Map::isActorWalkable(int x, int y) {
  return areCoordsValid(x, y) && tiles[x + y * width].material.passable && !tiles[x + y * width].actorOccupied;
}

// For allowing actors to path around other actors. May be better to combine into one method that takes a boolean
void Map::registerActorPose(int x, int y) {
  tiles[x + width * y].actorOccupied = true;
}

void Map::deregisterActorPose(int x, int y) {
  tiles[x + width * y].actorOccupied = false;
}

void Map::setMaterial(int x, int y, Material material) {
  tiles[x + y * width].material = material;
}

void Map::setDisintegrate(int x, int y, bool disintegrate) {
  tiles[x + y * width].disintegrate = disintegrate;
}

Material Map::getMaterial(int x, int y) {
  return tiles[x + y * width].material;
}

bool Map::getDisintegrate(int x, int y) {
  return tiles[x + y * width].disintegrate;
}

Damage Map::getDamaged(int x, int y) {
  return tiles[x + y * width].damage;
}

void Map::setDamaged(int x, int y, Damage d) {
  tiles[x + y * width].damage = d;
}

bool Map::getInUse(int x, int y) {
  return tiles[x + y * width].inUse;
}

void Map::setInUse(int x, int y, bool status) {
  tiles[x + y * width].inUse = status;
}

int Map::getWater(int x, int y) {
  return tiles[x + y * width].water;
}

void Map::setWater(int x, int y, int amount) {
  tiles[x + y * width].water = amount;
}

bool Map::areCoordsValid(int x, int y) {
  return x >= 0 && x < width && y >= 0 && y < height;
}

bool Map::sunExposure(int x, int y, int tickCount) {
  if (!areCoordsValid(x, y - 1)) return cos(tickCount / 150.0) > 0; // take into account day / night cycle
  if (!isActorWalkable(x, y - 1)) return false;
  return sunExposure(x, y - 1, tickCount);
}

void Map::render(tcod::Console &console, int cursorX, int cursorY, int tickCount) {

  // Updates light
  // TODO - make this work with arbirary light source tiles instead of just raycasting down from the top of the map
  // for (int i = 0; i < width; i++) {
    // for (int j = 0; j < height; j++) {
      // if (getTile(i, j)->material.passable) {
        // getTile(i, j)->light = 255 - 2 * j;
      // } else {
        // getTile(i, j)->light = 0;
        // break;
      // }
    // }
  // }
  // std::vector<std::pair<int, int>> lightSources = std::vector<std::pair<int, int>>();
  // for (int i = 0; i < width; i++) {
    // for (int j = 0; j < height; j++) {
      // if (getTile(i, j)->emitsLight) lightSources.push_back(std::pair<int, int>(i, j));
    // }
  // }
  // for (int i = 0; i < width; i++) {
    // for (int j = 0; j < height; j++) {
      // if (getTile(i, j)->material.passable) {
        // bool rayClear = true;
        // for (int sourceX = 0; sourceX < width; sourceX++) {
          // for (int sourceY = 0; sourceY < height; sourceY++) {
            // if (getTile(sourceX, sourceY)->emitsLight) {
              // TCOD_bresenham_data_t data;
              // int x = i, y = j, distance = 0;
              // TCOD_line_init_mt(i, j, sourceX, sourceY, &data);
              // do {
                // rayClear &= getTile(x, y)->material.passable;
                // distance++;
              // } while (!TCOD_line_step_mt(&x, &y, &data));
              // if (getTile(i, j)->light == 0 && rayClear) { getTile(i, j)->light = 255 - 5 * distance; }
            // }
          // }
        // }
      // }
    // }
  // }
  // for (int i = 0; i < width; i++) {
    // for (int j = 0; j < height; j++) {
      // getTile(i, j)->light = 255 - 3 * j;
    // }
  // }

  // Shade by depth and day-night cycle interval, with designated star sky tiles having a minimum brightness
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      int light = getTile(i, j)->star ? MAX(2 * (height - j) * (0.5 + cos(tickCount / 150.0)), 225) : 2 * (height - j) * (0.5 + cos(tickCount / 150.0));
      // int light = 2 * (height - j) * (0.5 + cos(tickCount / 150.0));
      getTile(i, j)->light = light > 255 ? 255 : light < 0 ? 0 : light;
    }
  }

  // Currently assumes a map where display width specifically is the entire width of the map.
  for (int i = 0; i < displayWidth; i++) {
    for (int j = cursorY - displayHeight / 2; j < cursorY + displayHeight / 2; j++) {
      if (areCoordsValid(i, j)) {
          Tile tile = *getTile(i, j);
          if (tile.water > 0 && (tickCount % 48 < 24 || tile.material.id == Material::VACUUM.id)) {
            TCOD_console_put_char_ex(console.get(), i, j - cursorY + displayHeight / 2,
              0x2588, TCOD_ColorRGB{0, 0, 0}, TCOD_ColorRGB{0, 0, 125});
          } else {
            // Flicker with blue background in water, and also shade based on depth
            // Ores need the background color and are unaffected, along with workstations with inUse flag
            int ch = 0;
            switch (getDamaged(i, j)) {
              case (INTACT): {
                TCOD_console_put_char_ex(console.get(), i, j - cursorY + displayHeight / 2,
                  tile.material.ch, tile.material.fg, tile.inUse ? TCOD_ColorRGB{255, 0, 0}
                  : tile.material.id == Material::COPPER_ORE.id || tile.material.id == Material::TIN_ORE.id
                  ? tile.material.bg : tile.water == 0
                  ? TCOD_ColorRGB{ tile.light, tile.light, tile.light } : TCOD_ColorRGB{0, 0, 125}); 
                break;
              }
              case (DAMAGED): {
                TCOD_console_put_char_ex(console.get(), i, j - cursorY + displayHeight / 2,
                  tile.material.chDamaged, tile.material.fg, tile.inUse ? TCOD_ColorRGB{255, 0, 0}
                  : tile.material.id == Material::COPPER_ORE.id || tile.material.id == Material::TIN_ORE.id
                  ? tile.material.bg : tile.water == 0
                  ? TCOD_ColorRGB{ tile.light, tile.light, tile.light } : TCOD_ColorRGB{0, 0, 125}); 
              }
              case (BROKEN): {
                TCOD_console_put_char_ex(console.get(), i, j - cursorY + displayHeight / 2,
                  tile.material.chBroken, tile.material.fg, tile.inUse ? TCOD_ColorRGB{255, 0, 0}
                  : tile.material.id == Material::COPPER_ORE.id || tile.material.id == Material::TIN_ORE.id
                  ? tile.material.bg : tile.water == 0
                  ? TCOD_ColorRGB{ tile.light, tile.light, tile.light } : TCOD_ColorRGB{0, 0, 125}); 
              }
            }
          }
      }
    }
  }
}

void Map::tick(int tickCount) {
  TCODRandom* rng = TCODRandom::getInstance();

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      // Disintegrate tile flag functionality
      if (getTile(i, j)->disintegrate && rng->getInt(0, 15) == 0) {
        if (getMaterial(i, j).id == Material::TRUNK.id) inventory->wood++;
        setMaterial(i, j, Material::VACUUM);
        getTile(i, j)->disintegrate = false;
      }
      // Plant seeds mature and are destroyed upon plot removal
      // TODO - add an age trait to tiles to check for plant maturity as well
      if (getMaterial(i, j).id == Material::CEREAL_SEED.id) {
        if (getMaterial(i, j + 1).id != Material::FARM_PLOT.id) {
          setMaterial(i, j, Material::VACUUM);
        } else if (sunExposure(i, j, tickCount) && rng->getInt(0, 1000) == 0) {
          setMaterial(i, j, Material::CEREAL_PLANT);
        }
      }
      // Plants are also deleted if the plot is destroyed
      if (getMaterial(i, j).id == Material::CEREAL_PLANT.id && getMaterial(i, j + 1).id != Material::FARM_PLOT.id) {
        setMaterial(i, j, Material::VACUUM);
      }
    }
  }

  // Rain
  if (tickCount % 10 == 0) {
    setWater(rng->getInt(0, width), 0, 1);
  }

  // Reset updated flag
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      getTile(i, j)->hasBeenUpdated = false;
    }
  }

  // Water flow
  for (int i = 0; i < width; i++) {
    for (int j = height; j >= 0; j--) {

      // Water with no neighbors may evaporate
      int evaporateXOffs[3] = {-1, 0, 1};
      int evaporateYOffs[3] = {-1, 0, 1};
      int waterNeighbors = 0;
      for (int evaporateX = 0; evaporateX < 3; evaporateX++) {
        for (int evaporateY = 0; evaporateY < 3; evaporateY++) {
          if (getWater(i + evaporateXOffs[evaporateX], j + evaporateYOffs[evaporateY]) > 0) {
            waterNeighbors++;
          }
        }
      }

      // TODO - reenable
      if (waterNeighbors < 2 && rng->getInt(0, 90) >= 0 && !isWalkable(i, j + 1)) setWater(i, j, 0);

      if (tickCount % 10 == (j % 10)) { // Update layers out of phase with one another
        if (!getTile(i, j)->hasBeenUpdated) {
          // Traverse neighboring tiles in the right order.
          // int xOffs[3] = {0, (j % 2 == 0) ? 1 : -1, (j % 2 == 0) ? -1 : 1};
          bool migratesHorizontally = true;//(areCoordsValid(i, j + 1) && getWater(i, j + 1) > 0);
          bool leftFirst = /*getWater(i + 1, j) > 0 && getWater(i - 1, j) == 0;*/rng->getInt(0, 1) == 0;
          int xOffs[3] = {0, migratesHorizontally ? ((leftFirst) ? -1 : 1) : 0, migratesHorizontally ? ((leftFirst) ? 1 : -1) : 0};
          // int xOffs[3] = {0, 1, -1};
          int yOffs[2] = {1, 0};
          for (int n = 0; n < 3; n++) {
            int xOff = xOffs[n];
            for (int m = 0; m < 2; m++) {
              int yOff = yOffs[m];
              if ((isWalkable(i + xOff, j + yOff) && getWater(i, j) > getWater(i + xOff, j + yOff) && !getTile(i + xOff, j + yOff)->hasBeenUpdated)) {
                getTile(i + xOff, j + yOff)->hasBeenUpdated = true;
                getTile(i, j)->hasBeenUpdated = true;
                setWater(i, j, 0);
                setWater(i + xOff, j + yOff, getMaterial(i + xOff, j + yOff).door ? 0 : 1); // Doors delete water so buildings don't flood
              } else if (!areCoordsValid(i + xOff, j + yOff)) { // Water can flow off map
                getTile(i, j)->hasBeenUpdated = true;
                setWater(i, j, 0);
              }
            }
          }
        }
      }
    }
  }
}
