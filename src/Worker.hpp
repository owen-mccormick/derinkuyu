#pragma once

#include <libtcod.hpp>
#include "Actor.hpp"
#include "Order.hpp"
#include <queue>

class Worker : public Actor {
  public:
    Map* map;
    Order order;
    Worker(Map* map, int x, int y);
    void act(int tickCount);
};
