#include <libtcod.hpp>
#include "Worker.hpp"
#include "Map.hpp"
#include <queue>
#include <iostream>
#include <cassert>
#include "Order.hpp"
#include "AStar.hpp"

Worker::Worker(Map* map, int x, int y) : Actor(x, y, '@', TCOD_ColorRGB{255, 255, 0}), order(Order(OrderType::IDLE, 0, 0, 0)), map(map) {};

void Worker::act(int tickCount) {
  if (tickCount % 2 == 0) {
    // Gravity (may make sense to implement this somewhere else)
    // if (order.type == OrderType::IDLE) std::cout << "Idle" << std::endl;
    // if (order.type == OrderType::MOVE) std::cout << "MOVE" << std::endl;
    if (map->isActorWalkable(getX(), getY() + 1) && !map->getMaterial(getX(), getY()).climbable) {
      moveTo(map, getX(), getY() + 1);
    } else {
      switch (order.type) {
        case OrderType::MOVE: {
          AStar astar = AStar(map, getX(), getY(), order.x, order.y);
          if (astar.calculate()) {
            moveTo(map, astar.walk());
          } else {
            order = Order(OrderType::IDLE, 0, 0, 0);
          }
          if (getX() == order.x && getY() == order.y) {
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
