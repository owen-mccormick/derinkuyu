#pragma once
#include <vector>

class Map;
class Actor;

class Engine {
  public:
    Engine();
    ~Engine();
    void update();
    void render();
    std::vector<Actor*> actors;
    Actor* player;
    Map* map;
};

extern Engine engine;
