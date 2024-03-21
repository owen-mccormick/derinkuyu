#pragma once

#include <libtcod.hpp>
#include "Actor.hpp"
#include "Order.hpp"
#include <queue>

class Worker : public Actor {
  public:
    Order order;
    // Constructor should probably be in cpp file - ?
    Worker(int x, int y) : Actor(x, y, 0x40, TCOD_ColorRGB{255, 255, 0}, TCOD_ColorRGB{0, 0, 0}), order(Order(OrderType::IDLE, 0, 0, 0)) {};
    void act(Map* map, int tickCount, std::priority_queue<Order> orders);
};
