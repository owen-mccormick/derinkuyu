#pragma once

// Something a worker is assigned to do at a certain priority
enum class OrderType {
  MOVE,
  BUILD,
  DIG,
  IDLE
};

struct Order {
  OrderType type;
  int priority, x, y;
  Order(OrderType type, int priority, int x, int y) : type(type), priority(priority), x(x), y(y) {};
};

// Comparator struct for priority queueing pointers
struct compareOrders {
  bool operator() (const Order a, const Order b) const {
    return a.priority < b.priority;
  }
};
