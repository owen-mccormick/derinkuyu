#include <libtcod.hpp>

#include "Map.hpp"
#include <algorithm>

int tickCount = 0;

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
      setMaterial(x, y, Material::WOOD);
    } while (!TCOD_line_step_mt(&x, &y, &data));
  }
  // Leaves
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < 10; j++) {
      for (int xOff = -1; xOff < 2; xOff++) {
        for (int yOff = -1; yOff < 2; yOff++) {
          if (areCoordsValid(i + xOff, j + yOff) && getMaterial(i + xOff, j + yOff) == Material::WOOD && getMaterial(i, j) != Material::WOOD) {
            setMaterial(i, j, Material::LEAVES);
          }
        }
      }
    }
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
    y = 0;
  } while (!isWalkable(x, y));
  return {x, y};
}

Map::~Map() {
  delete [] tiles;
}

void Map::setWalkable(int x, int y, bool walk) {
  tiles[x + y * width].material = walk ? Material::VACUUM : Material::ROCK;
}

bool Map::isWalkable(int x, int y) {
  Material material = tiles[x + y * width].material;
  return material == Material::VACUUM || material == Material::WOOD || material == Material::LEAVES;
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
  return x > 0 && x < width && y > 0 && y < height;
}

void Map::render(tcod::Console &console, int playerx, int playery) {
  // Currently assumes a map where display width specifically is the entire width of the map.
  for (int i = 0; i < displaywidth; i++) {
    for (int j = playery - displayheight / 2; j < playery + displayheight / 2; j++) {
      if (areCoordsValid(i, j)) {
        if (!isWalkable(i, j)) {
        } else {
          if (getTile(i, j)->water > 0) {
            // TCOD_console_put_char(console.get(), i, j, 0x2591, TCOD_BKGND_NONE);
            TCOD_console_put_char_ex(console.get(), i, j - playery + displayheight / 2,
              0x2591, TCOD_ColorRGB{0, 0, 255}, TCOD_ColorRGB{0, 0, 0});
          } else {
            TCOD_console_put_char(console.get(), i, j - playery + displayheight / 2, 0x00, TCOD_BKGND_NONE);
          }
        }
        if (getTile(i, j)->water > 0) {
          TCOD_console_put_char_ex(console.get(), i, j - playery + displayheight / 2,
            0x2591, TCOD_ColorRGB{0, 0, 255}, TCOD_ColorRGB{0, 0, 0});
        } else {
          switch (getTile(i, j)->material) {
            case Material::VACUUM: {
              TCOD_console_put_char(console.get(), i, j - playery + displayheight / 2, 0x00, TCOD_BKGND_NONE);
              break;
            }
            case Material::ROCK: {
              TCOD_console_put_char(console.get(), i, j - playery + displayheight / 2, 0x2593, TCOD_BKGND_NONE);
              break;
            }
            case Material::WOOD: {
              TCOD_console_put_char_ex(console.get(), i, j - playery + displayheight / 2,
                0x2551, TCOD_ColorRGB{166, 42, 42}, TCOD_ColorRGB{0, 0, 0});
              break;
            }
            case Material::LEAVES: {
              TCOD_console_put_char_ex(console.get(), i, j - playery + displayheight / 2,
                '#', TCOD_ColorRGB{0, 255, 0}, TCOD_ColorRGB{0, 0, 0});
              break;
            }
          }
        }
      }
    }
  }
}

void Map::tick() {
  // Rain
  tickCount++;
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
