#include <libtcod.hpp>
#include "Worker.hpp"
#include "Map.hpp"
#include <queue>
#include <iostream>
#include <cassert>
#include "Order.hpp"
#include "AStar.hpp"
#include "float.h"
#include "math.h"

Worker::Worker(Map* map, std::priority_queue<Order, std::vector<Order>, compareOrders>* taskQueue, Inventory* inventory, int x, int y)
  : Actor(x, y, '@', TCOD_ColorRGB{255, 255, 0}), order(Order(OrderType::IDLE, 0, 0, 0, 0, 0)),
  map(map), taskQueue(taskQueue), inventory(inventory), blockBreakTime(0) {};

// TODO - can probably be cleaned up to some extent - delete redundant code
void Worker::moveTowardDestination() {
  if (
    order.type == OrderType::DIG || order.type == OrderType::BUILD || order.type == OrderType::TILL
    || order.type == OrderType::PLANT || order.type == OrderType::HARVEST
  ) {
    // Reject invalid order designations
    switch (order.type) {
      case OrderType::DIG: {
        // Can't dig passable tiles
        if (map->getMaterial(order.interestX, order.interestY).passable) {
          return;
        }
        break;
      }
      case OrderType::BUILD: {
        // Can't build in non-empty tiles
        // Also make sure we don't build an impassable tile on top of another actor
        if (order.interestMaterial.climbable || order.interestMaterial.door ? !map->isWalkable(order.interestX, order.interestY) : !map->isActorWalkable(order.interestX, order.interestY)) {
          order = Order(OrderType::IDLE, 0, 0, 0, 0, 0);
          // std::cout << "Rejected build in occupied cell" << std::endl;
          return;
        }
        break;
      }
      case OrderType::TILL: {
        if (map->getMaterial(order.interestX, order.interestY).id != Material::DIRT.id) {
          order = Order(OrderType::IDLE, 0, 0, 0, 0, 0);
          // std::cout << "Rejected till on unfarmable tile" << std::endl;
          return;
        }
        break; 
      }
      case OrderType::PLANT: {
        if (map->getMaterial(order.interestX, order.interestY).id != Material::FARMPLOT.id) {
          order = Order(OrderType::IDLE, 0, 0, 0, 0, 0);
          // std::cout << "Rejected plant on untilled tile" << std::endl;
          return;
        }
        break;
      }
      case OrderType::HARVEST: {
        // TODO - identity as plant should probably be made into a boolean flag for materials once more harvestable plants get added
        if (map->getMaterial(order.interestX, order.interestY).id != Material::CEREALPLANT.id) {
          order = Order(OrderType::IDLE, 0, 0, 0, 0, 0);
          // std::cout << "Rejected harvesting of non-plant" << std::endl;
          return;
        }
        break;
      }
      default: break;
    }
    // Figure out which neighboring non-floating tiles are accessible and go for the closest one
    double minDist = DBL_MAX;
    int xOff[3] = {-1, 0, 1};
    int yOff[3] = {-1, 0, 1};
    for (int n = 0; n < 3; n++) {
      for (int m = 0; m < 3; m++) {
        AStar astar = AStar(map, getX(), getY(), order.interestX + xOff[n], order.interestY + yOff[m]);
        if (
            astar.calculate()
            // non-floating
            // && (!map->isWalkable(order.interestX + xOff[n], order.interestY + yOff[m] + 1) || map->getMaterial(order.interestX + xOff[n], order.interestY + yOff[m]).climbable)
            // && !(order.interestX + xOff[n] == getX() && order.interestY + yOff[m] == getY())
            // && !(xOff[n] == 0 && yOff[m] == -1)
          ) {
          if (hypot(order.interestX - getX(), order.interestY - getY()) <= sqrt(2)) {
            minDist = 0;
            order.pathX = getX();
            order.pathY = getY();
          } else {
            order.pathX = order.interestX + xOff[n];
            order.pathY = order.interestY + yOff[m];
            minDist = hypot(order.pathX - getX(), order.pathY - getY());
          }
        }
      }
    }
    if (minDist == DBL_MAX) {
      // Drop and demote; see below
      order.priority -= 1;
      taskQueue->push(order);
      order = Order(OrderType::IDLE, 0, 0, 0, 0, 0);
    }
  } else if (order.type == OrderType::CHOP) {
    // TODO - remove duplicated logic here
    // Can't chop non-tree tiles
    if (map->getMaterial(order.interestX, order.interestY).id != Material::TRUNK.id) {
      order = Order(OrderType::IDLE, 0, 0, 0, 0, 0);
      return;
    }
    // Shift interest tile to base of tree
    do {
      if (map->getMaterial(order.interestX - 1, order.interestY).id == Material::TRUNK.id) {
        order.interestX -= 1;
      } else if (map->getMaterial(order.interestX - 1, order.interestY + 1).id == Material::TRUNK.id) {
        order.interestX -= 1;
        order.interestY += 1;
      } else if (map->getMaterial(order.interestX - 1, order.interestY - 1).id == Material::TRUNK.id) {
        order.interestX -= 1;
        order.interestY -= 1;
      } else {
        break;
      }
    } while (true);
    order.pathX = order.interestX;
    order.pathY = order.interestY;
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
          blockBreakTime++;
          if (blockBreakTime >= 5) {
            switch (map->getDamaged(order.interestX, order.interestY)) {
              case (Damage::INTACT): {
                map->setDamaged(order.interestX, order.interestY, Damage::DAMAGED);
                blockBreakTime = 0;
                break;
              }
              case (Damage::DAMAGED): {
                map->setDamaged(order.interestX, order.interestY, Damage::BROKEN);
                blockBreakTime = 0;
                break;
              }
              case (Damage::BROKEN): {
                map->setMaterial(order.interestX, order.interestY, Material::VACUUM);
                order = Order(OrderType::IDLE, 0, 0, 0, 0, 0);
                blockBreakTime = 0;
                break;
              }
            }
          }
        } else {
          blockBreakTime = 0;
        }
        break;
      }
      case OrderType::CHOP: {
        moveTowardDestination();
        if (getX() == order.pathX && getY() == order.pathY) {
          recursiveTreeDelete(order.interestX, order.interestY);
          order = Order(OrderType::IDLE, 0, 0, 0, 0, 0);
        }
        break;
      }
      case OrderType::BUILD: {
        moveTowardDestination();
        if (getX() == order.pathX && getY() == order.pathY) {
          if (inventory->wood > 0) {
            map->setMaterial(order.interestX, order.interestY, order.interestMaterial);
            inventory->wood--;
          }
          order = Order(OrderType::IDLE, 0, 0, 0, 0, 0);
        }
        break;
      }
      case OrderType::TILL: {
        moveTowardDestination();
        if (getX() == order.pathX && getY() == order.pathY) {
          map->setMaterial(order.interestX, order.interestY, Material::FARMPLOT);
          order = Order(OrderType::IDLE, 0, 0, 0, 0, 0);
        }
        break;
      }
      case OrderType::PLANT: {
        moveTowardDestination();
        if (getX() == order.pathX && getY() == order.pathY) {
          if (map->isWalkable(order.interestX, order.interestY - 1) && inventory->cerealSeed > 0) {
            map->setMaterial(order.interestX, order.interestY - 1, Material::CEREALSEED);
            inventory->cerealSeed--;

          }
          order = Order(OrderType::IDLE, 0, 0, 0, 0, 0);
        }
        break;
      }
      case OrderType::HARVEST: {
        moveTowardDestination();
        if (getX() == order.pathX && getY() == order.pathY) {
          if (map->getMaterial(order.interestX, order.interestY).id == Material::CEREALPLANT.id) {
            map->setMaterial(order.interestX, order.interestY, Material::VACUUM);
            inventory->cerealSeed = inventory->cerealSeed + 2;
            inventory->cerealGrain++;
          }
          order = Order(OrderType::IDLE, 0, 0, 0, 0, 0);
        }
        break;
      }
      default: {
        break;
      }
    }
  }
}

void Worker::recursiveTreeDelete(int x, int y) {
  // if (map->getMaterial(x, y).id == Material::TRUNK.id) inventory->wood++;
  map->setDisintegrate(x, y, true);
  int xOff[3] = {-1, 0, 1};
  int yOff[3] = {-1, 0, 1};
  for (int m = 0; m < 3; m++) {
    for (int n = 0; n < 3; n++) {
      if (
        (xOff[m] != 0 || yOff[n] != 0)
        && (map->getMaterial(x + xOff[m], y + yOff[n]).id == Material::TRUNK.id || map->getMaterial(x + xOff[m], y + yOff[n]).id == Material::LEAVES.id)
        && !map->getDisintegrate(x + xOff[m], y + yOff[n])
      ) {
        
        recursiveTreeDelete(x + xOff[m], y + yOff[n]);
      }
    }
  }
}
