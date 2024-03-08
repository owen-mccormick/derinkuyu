#pragma once
#include <libtcod.hpp>

enum class Material {
  VACUUM,
  ROCK,
  WOOD,
  LEAVES
};

struct Tile {
  Material material;
  // Water amount between 0 and 100
  int water;
  Tile() : material(Material::VACUUM), water(0) {};
};

class Map {
  public:
    Map(int width, int height, int displaywidth, int displayheight);
    ~Map();
    void render(tcod::Console& console, int playerx, int playery);
    void setWalkable(int x, int y, bool walk);
    bool isWalkable(int x, int y);
    void setMaterial(int x, int y, Material material);
    Material getMaterial(int x, int y);
    int getWater(int x, int y);
    void setWater(int x, int y, int amount);
    void tick();
    bool areCoordsValid(int x, int y);
    std::pair<int, int> placePlayer();
    int width, height, displaywidth, displayheight;
  protected:
    Tile* tiles;
    Tile* getTile(int x, int y);
};
