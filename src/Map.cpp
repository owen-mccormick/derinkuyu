#include <libtcod.hpp>
#include "Map.hpp"
#include <algorithm>
#include <iostream>

// Materials (maybe should be done in .hpp)
const Material Material::VACUUM = Material(true, false, 0x00, 0, TCOD_ColorRGB{0, 0, 0}, TCOD_ColorRGB{250, 250, 250});
const Material Material::ROCK = Material(false, false, 0x2593, 1, TCOD_ColorRGB{100, 85, 75}, TCOD_ColorRGB{250, 250, 250});
const Material Material::TRUNK = Material(true, false, 0x2551, 2, TCOD_ColorRGB{166, 42, 42}, TCOD_ColorRGB{250, 250, 250}); 
const Material Material::LEAVES = Material(true, false, '#', 3, TCOD_ColorRGB{0, 255, 0}, TCOD_ColorRGB{250, 250, 250});
const Material Material::DIRT = Material(false, false, 0x2593, 4, TCOD_ColorRGB{155, 118, 83}, TCOD_ColorRGB{250, 250, 250});
const Material Material::GRASS = Material(true, false, '=', 5, TCOD_ColorRGB{0, 255, 0}, TCOD_ColorRGB{250, 250, 250});
const Material Material::LADDER = Material(true, true, 'H', 6, TCOD_ColorRGB{166, 42, 42}, TCOD_ColorRGB{250, 250, 250});

Map::Map(int width, int height, int displayWidth, int displayHeight) : width(width),
    height(height), displayWidth(displayWidth), displayHeight(displayHeight) {

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
  for (int k = 0; k < 3; k++) {
    TCOD_bresenham_data_t data;
    int x = rng->getInt(0, width), y = 13;
    TCOD_line_init_mt(x, y, x + rng->getInt(-1, 1), 5, &data);
    do {
      setMaterial(x, y, Material::TRUNK);
    } while (!TCOD_line_step_mt(&x, &y, &data));
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
}

Tile* Map::getTile(int x, int y) {
  return &tiles[x + y * width];
}

std::pair<int, int> Map::placePlayer() {
  TCODRandom* rng = TCODRandom::getInstance();
  int x, y;
  do {
    x = rng->getInt(0, width);
    y = 9;
  } while (!isWalkable(x, y));
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

Material Map::getMaterial(int x, int y) {
  return tiles[x + y * width].material;
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
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      getTile(i, j)->light = 255 - 3 * j;
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
            TCOD_console_put_char_ex(console.get(), i, j - cursorY + displayHeight / 2,
              tile.material.ch, tile.material.fg, tile.water == 0
              ? TCOD_ColorRGB{ tile.light, tile.light, tile.light } : TCOD_ColorRGB{0, 0, 125}); 
          }
      }
    }
  }
}

void Map::tick(int tickCount) {
  // Rain
  TCODRandom* rng = TCODRandom::getInstance();
  if (tickCount % 8 == 0) {
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
      if (waterNeighbors < 2 && rng->getInt(0, 90) == 0 && !isWalkable(i, j + 1)) setWater(i, j, 0);

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
                // int flowAmount = std::min(getWater(i, j), 100 - getWater(i + xOff, j + yOff));
                // double flowAmount = (getWater(i, j) - getWater(i + xOff, j + yOff)) / 2;
                int flowAmount = getWater(i, j) > 0 ? 1 : 0;
                // double flowAmount = (yOff == 0 ? (getWater(i, j) - getWater(i + xOff, j + yOff)) / 2 : std::min(getWater(i, j), 100 - getWater(i + xOff, j + yOff)));
                setWater(i, j, 0);
                setWater(i + xOff, j + yOff, 1);
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
