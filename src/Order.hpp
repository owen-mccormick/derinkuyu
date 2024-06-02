#pragma once

// Something a worker is assigned to do at a certain priority
enum class OrderType {
  MOVE,
  DIG,
  CHOP,
  BUILD,
  IDLE,
  TILL,
  PLANT,
  HARVEST,
  SMELT,
  MILL,
  TRADE,
  SLEEP // TODO - unimplemented
};

// May be cleaner way to accomplish this
enum class TradeType {
  BUY_WOOD,
  SELL_BRONZE,
  SELL_FLOUR
};

struct Order {
  OrderType type;
  int priority, pathX, pathY, interestX, interestY;
  Material interestMaterial; // Optional; used in building
  TradeType interestTrade; // Used only to specify what type of trade order
  Order(OrderType type, int priority, int pathX, int pathY, int interestX, int interestY, Material interestMaterial)
    : type(type), priority(priority), pathX(pathX), pathY(pathY), interestX(interestX), interestY(interestY), interestMaterial(interestMaterial), interestTrade(TradeType::BUY_WOOD) {};
  Order(OrderType type, int priority, int pathX, int pathY, int interestX, int interestY)
    : type(type), priority(priority), pathX(pathX), pathY(pathY), interestX(interestX), interestY(interestY), interestMaterial(Material::VACUUM), interestTrade(TradeType::BUY_WOOD) {};
  Order(OrderType type, int priority, int pathX, int pathY, int interestX, int interestY, TradeType tradeType)
    : type(type), priority(priority), pathX(pathX), pathY(pathY), interestX(interestX), interestY(interestY), interestMaterial(Material::VACUUM), interestTrade(tradeType) {};
};

// Comparator struct for priority queueing pointers
struct compareOrders {
  bool operator() (const Order a, const Order b) const {
    return a.priority < b.priority;
  }
};
