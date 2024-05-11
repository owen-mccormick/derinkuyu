#include <libtcod.hpp>
#include "Worker.hpp"
#include "Map.hpp"
#include <queue>
#include <iostream>
#include <cassert>
#include "Order.hpp"
#include "AStar.hpp"

Worker::Worker(int x, int y) : Actor(x, y, 0x40, TCOD_ColorRGB{255, 255, 0}, TCOD_ColorRGB{0, 0, 0}), order(Order(OrderType::IDLE, 0, 0, 0)) {};

void Worker::act(Map* map, int tickCount, std::priority_queue<Order> orders, TCODMap* navgrid) {
  if (tickCount % 2 == 0) {
    // Gravity (may make sense to implement this somewhere else)
    // if (order.type == OrderType::IDLE) std::cout << "Idle" << std::endl;
    // if (order.type == OrderType::DIG) std::cout << "Dig" << std::endl;
    if (map->areCoordsValid(x, y + 1) && map->isWalkable(x, y + 1) && !map->getMaterial(x, y).climbable) {
      y++;
    } else if (order.type == OrderType::IDLE && !orders.empty()) {
      order = orders.top();
      orders.pop();
    } else {
      switch (order.type) {
        case OrderType::DIG: {
          AStar astar = AStar(map, x, y, order.x, order.y);
          if (astar.calculate()) {
            std::pair<int, int> moveTo = astar.walk();
            x = moveTo.first;
            y = moveTo.second;
          }
          if (x == order.x && y == order.y) {
            order = Order(OrderType::IDLE, 0, 0, 0);
          }
          break;
        }
        default: {
          break;
        }
      }
    }
  }
}
