#pragma once
#include <libtcod.hpp>
#include <vector>

class Material {
  public:
    // Statically declared types of materials; constructor is private. This is all done to do something similar to a Java-style enum.
    // There may be a better way to do comparison rather than by id field
    static const Material VACUUM;
    static const Material ROCK;
    static const Material TRUNK;
    static const Material LEAVES;
    static const Material DIRT;
    static const Material GRASS;
    static const Material LADDER;
    static const Material WHEEL;
    static const Material PLANK;
    static const Material CANOPY;
    static const Material DOOR;
    std::string name;
    bool passable;
    bool climbable;
    bool door;
    int ch;
    int id;
    TCOD_ColorRGB fg;
    TCOD_ColorRGB bg;
  private:
    Material(std::string name, bool pass, bool climb, bool door, int character, int id, const TCOD_ColorRGB& foreground, const TCOD_ColorRGB& background)
      : passable(pass), climbable(climb), door(door), ch(character), id(id), fg(foreground), bg(background) {}
    Material(std::string name, bool pass, bool climb, int character, int id, const TCOD_ColorRGB& foreground, const TCOD_ColorRGB& background)
      : passable(pass), climbable(climb), door(false), ch(character), id(id), fg(foreground), bg(background) {}
};

struct Tile {
  Material material;
  int water; // 0 to 100
  uint8_t light; // 0 to 255
  bool actorOccupied;
  bool emitsLight;
  bool hasBeenUpdated;
  Tile() : material(Material::VACUUM), water(0), light(0), actorOccupied(false), emitsLight(false), hasBeenUpdated(false) {};
};

class Map {
  public:
    Map(int width, int height, int displayWidth, int displayHeight);
    ~Map();
    void render(tcod::Console& console, int cursorX, int cursorY, int tickCount);
    void setWalkable(int x, int y, bool walk);
    bool isWalkable(int x, int y);
    bool isActorWalkable(int x, int y);
    void setMaterial(int x, int y, Material material);
    Material getMaterial(int x, int y);
    int getWater(int x, int y);
    void setWater(int x, int y, int amount);
    void tick(int tickCount);
    bool areCoordsValid(int x, int y);
    void registerActorPose(int x, int y);
    void deregisterActorPose(int x, int y);
    std::pair<int, int> placePlayer();
    int width, height, displayWidth, displayHeight;
  protected:
    Tile* tiles;
    Tile* getTile(int x, int y);
    int wagonX;
};
