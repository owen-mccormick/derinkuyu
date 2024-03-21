#include <libtcod.hpp>
#include "Worker.hpp"
#include "Map.hpp"
#include <queue>
#include <iostream>
#include "Order.hpp"
#include <cassert>

// Worker::Worker(int x, int y) : Actor(x, y, 0x40, TCOD_ColorRGB{255, 255, 0}, TCOD_ColorRGB{0, 0, 0}) {};

void Worker::act(Map* map, int tickCount, std::priority_queue<Order> orders) {
  if (tickCount % 2 == 0) {
    // Gravity (may make sense to implement this somewhere else)
    if (order.type == OrderType::IDLE) std::cout << "Idle" << std::endl;
    // if (order.type == OrderType::DIG) std::cout << "Dig" << std::endl;
    if (map->areCoordsValid(x, y + 1) && map->isWalkable(x, y + 1)) {
      y++;
    } else if (order.type == OrderType::IDLE && !orders.empty()) {
      order = orders.top();
      orders.pop();
    } else {
      switch (order.type) {
        case OrderType::DIG: {
          TCODMap* navgrid = new TCODMap(map->width, map->height);
          for (int i = 0; i < map->width; i++) {
            for (int j = 0; j < map->height; j++) {
              navgrid->setProperties(i, j, map->isWalkable(i, j), map->isWalkable(i, j));
            }
          }
          TCODDijkstra dijkstra = TCODDijkstra(navgrid);
          dijkstra.compute(x, y);
          // assert(dijkstra.getDistance(10, 10) != -1.0f);
          int newX = 0;
          int newY = 0;
          if (dijkstra.setPath(order.x, order.y) && dijkstra.walk(&newX, &newY)) {
            // std::cout << "Walk success: " << dijkstra.walk(&newX, &newY) << std::endl;
            // std::cout << "New X: " << newX << std::endl;
            // std::cout << "New Y: " << newY << std::endl;
            std::cout << "Walking..." << std::endl;
            if (x < newX /*&& map->isWalkable(x + 1, y)*/) x++;
            if (x > newX /*&& map->isWalkable(x - 1, y)*/) x--;
            if (y > newY /*&& map->isWalkable(x, y - 1)*/) y--;
            if (y < newY /*&& map->isWalkable(x, y + 1)*/) y++;
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
