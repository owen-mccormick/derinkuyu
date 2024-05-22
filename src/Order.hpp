#pragma once

// Something a worker is assigned to do at a certain priority
enum class OrderType {
  MOVE,
  DIG,
  CHOP,
  IDLE
};

struct Order {
  OrderType type;
  int priority, pathX, pathY, interestX, interestY;
  Order(OrderType type, int priority, int pathX, int pathY, int interestX, int interestY)
    : type(type), priority(priority), pathX(pathX), pathY(pathY), interestX(interestX), interestY(interestY) {};
};

// Comparator struct for priority queueing pointers
struct compareOrders {
  bool operator() (const Order a, const Order b) const {
    return a.priority < b.priority;
  }
};
