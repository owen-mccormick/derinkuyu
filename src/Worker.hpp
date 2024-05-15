#pragma once

#include <libtcod.hpp>
#include "Actor.hpp"
#include "Order.hpp"
#include <queue>

class Worker : public Actor {
  public:
    Order order;
    Worker(int x, int y);
    void act(Map* map, int tickCount, std::priority_queue<Order> orders);
};
