#pragma once

// Something a worker is assigned to do at a certain priority
enum class OrderType {
  MOVE,
  BUILD,
  IDLE
};

struct Order {
  OrderType type;
  int priority, x, y;
  Order(OrderType type, int priority, int x, int y) : type(type), priority(priority), x(x), y(y) {};
  // bool operator<(const Order& a) const {
    // return this->priority < a.priority;
  // }
};

inline bool operator<(const Order& a, const Order& b) {
  return a.priority < b.priority;
}
