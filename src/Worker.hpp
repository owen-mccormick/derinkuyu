#pragma once

#include <libtcod.hpp>
#include "Actor.hpp"
#include "Order.hpp"
#include "Inventory.hpp"
#include <queue>

class Worker : public Actor {
  public:
    Map* map;
    std::priority_queue<Order, std::vector<Order>, compareOrders>* taskQueue;
    Inventory* inventory;
    Worker(Map* map, std::priority_queue<Order, std::vector<Order>, compareOrders>* taskQueue, Inventory* inventory, int x, int y);
    void act(int tickCount);
  protected:
    Order order;
    void moveTowardDestination(); // Movement towards goal common to multiple action types
    void recursiveTreeDelete(int x, int y);
};
