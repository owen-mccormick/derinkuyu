#include <libtcod.hpp>
#include "Map.hpp"
#include <algorithm>

// Materials (maybe should be done in .hpp)
const Material Material::VACUUM = Material(true, false, 0x00, 0, TCOD_ColorRGB{0, 0, 0}, TCOD_ColorRGB{0, 0, 0});
const Material Material::ROCK = Material(false, false, 0x2593, 1, TCOD_ColorRGB{128, 128, 128}, TCOD_ColorRGB{0, 0, 0});
const Material Material::TRUNK = Material(true, false, 0x2551, 2, TCOD_ColorRGB{166, 42, 42}, TCOD_ColorRGB{0, 0, 0}); 
const Material Material::LEAVES = Material(true, false, '#', 3, TCOD_ColorRGB{0, 255, 0}, TCOD_ColorRGB{0, 0, 0});
const Material Material::DIRT = Material(false, false, 0x2593, 4, TCOD_ColorRGB{155, 118, 83}, TCOD_ColorRGB{0, 0, 0});
const Material Material::GRASS = Material(true, false, '=', 5, TCOD_ColorRGB{0, 255, 0}, TCOD_ColorRGB{0, 0, 0});
const Material Material::LADDER = Material(true, true, 'H', 6, TCOD_ColorRGB{166, 42, 42}, TCOD_ColorRGB{0, 0, 0});

Map::Map(int width, int height, int displaywidth, int displayheight) : width(width),
    height(height), displaywidth(displaywidth), displayheight(displayheight) {

  tiles = new Tile[width * height];
  TCODRandom* rng = TCODRandom::getInstance();

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
  // Currently assumes a map where display width specifically is the entire width of the map.
  for (int i = 0; i < displaywidth; i++) {
    for (int j = cursorY - displayheight / 2; j < cursorY + displayheight / 2; j++) {
      if (areCoordsValid(i, j)) {
          Tile tile = *getTile(i, j);
          if (tile.water > 0 && (tickCount % 48 < 24 || tile.material.id == Material::VACUUM.id)) {
            TCOD_console_put_char_ex(console.get(), i, j - cursorY + displayheight / 2,
              0x2588, TCOD_ColorRGB{0, 0, 0}, TCOD_ColorRGB{0, 0, 255});
          } else {
            TCOD_console_put_char_ex(console.get(), i, j - cursorY + displayheight / 2,
              tile.material.ch, tile.material.fg, tile.water == 0 ? tile.material.bg : TCOD_ColorRGB{0, 0, 255}); // Flicker with blue background in water
          }
      }
    }
  }
}

void Map::tick(int tickCount) {
  // Rain
  TCODRandom* rng = TCODRandom::getInstance();
  if (tickCount % 20 == 0) {
    setWater(rng->getInt(0, width), 0, 1);
  }

  // Water flow
  if (tickCount % 5 == 0) {
    for (int i = 0; i < width; i++) {
      for (int j = height; j >= 0; j--) {
        // Traverse neighboring tiles in the right order.
        int xOffs[3] = {0, 1, -1};
        int yOffs[2] = {1, 0};
        for (int n = 0; n < 3; n++) {
          int xOff = xOffs[n];
          for (int m = 0; m < 2; m++) {
            int yOff = yOffs[m];
            if (isWalkable(i + xOff, j + yOff) && getWater(i, j) > getWater(i + xOff, j + yOff) && i + xOff >= 0
                && i + xOff <= width && j + yOff >= 0 && j + yOff <= height) {
              // int flowAmount = std::min(getWater(i, j), 100 - getWater(i + xOff, j + yOff));
              // double flowAmount = (getWater(i, j) - getWater(i + xOff, j + yOff)) / 2;
              int flowAmount = getWater(i, j) > 0 ? 1 : 0;
              // double flowAmount = (yOff == 0 ? (getWater(i, j) - getWater(i + xOff, j + yOff)) / 2 : std::min(getWater(i, j), 100 - getWater(i + xOff, j + yOff)));
              setWater(i, j, getWater(i, j) - flowAmount);
              setWater(i + xOff, j + yOff, getWater(i + xOff, j + yOff) + flowAmount);
            }
          }
        }
      }
    }
  }
}
