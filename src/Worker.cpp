#include <libtcod.hpp>
#include "Worker.hpp"
#include "Map.hpp"
#include <queue>
#include <iostream>
#include <cassert>
#include "Order.hpp"
#include "AStar.hpp"

Worker::Worker(Map* map, std::priority_queue<Order, std::vector<Order>, compareOrders>* taskQueue, int x, int y)
  : Actor(x, y, '@', TCOD_ColorRGB{255, 255, 0}), order(Order(OrderType::IDLE, 0, 0, 0, 0, 0)), map(map), taskQueue(taskQueue) {};

// TODO - can probably be cleaned up to some extent
void Worker::moveTowardDestination() {
  if (order.type == OrderType::DIG) {
    // Figure out which neighboring tiles are accessible if going to a dig
    bool foundOpen = false;
    int xOff[3] = {-1, 0, 1};
    int yOff[3] = {-1, 0, 1};
    for (int n = 0; n < 3; n++) {
      for (int m = 0; m < 3; m++) {
        AStar astar = AStar(map, getX(), getY(), order.interestX + xOff[n], order.interestY + yOff[m]);
        if (astar.calculate()) {
          order.pathX = order.interestX + xOff[n];
          order.pathY = order.interestY + yOff[m];
          foundOpen = true;
          break;
        }
      }
    }
    if (!foundOpen) {
      // Drop and demote; see below
      order.priority -= 1;
      taskQueue->push(order);
      order = Order(OrderType::IDLE, 0, 0, 0, 0, 0);
    }
  }
  AStar astar = AStar(map, getX(), getY(), order.pathX, order.pathY);
  if (astar.calculate()) {
    moveTo(map, astar.walk());
  } else {
    // Task destination is inaccessible, so we demote the priority of our current task, put it back in the to-do list, and go idle
    order.priority -= 1;
    taskQueue->push(order);
    order = Order(OrderType::IDLE, 0, 0, 0, 0, 0);
  }
}

void Worker::act(int tickCount) {
  if (tickCount % 2 == 0) {
    switch (order.type) {
      case OrderType::IDLE: {
        // Gravity (may make sense to implement this somewhere else)
        if (map->isActorWalkable(getX(), getY() + 1) && !map->getMaterial(getX(), getY()).climbable) {
          moveTo(map, getX(), getY() + 1);
        }
        // Pick up a task
        if (taskQueue->size() > 0) {
          order = taskQueue->top();
          taskQueue->pop();
        }
        break;
      }
      case OrderType::MOVE: {
        moveTowardDestination();
        // Complete condition for the move
        if (getX() == order.pathX && getY() == order.pathY) {
          order = Order(OrderType::IDLE, 0, 0, 0, 0, 0);
        }
        break;
      }
      case OrderType::DIG: {
        moveTowardDestination();
        // Complete condition for dig
        if (getX() == order.pathX && getY() == order.pathY) {
          map->setMaterial(order.interestX, order.interestY, Material::VACUUM);
          order = Order(OrderType::IDLE, 0, 0, 0, 0, 0);
        }
      }
      default: {
        break;
      }
    }
  }
}
