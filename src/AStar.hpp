#pragma once

#include <vector>
#include <queue>
#include "Map.hpp"

enum class NodeState {
  OPEN,
  CLOSED,
  HIDDEN
};

// Represents a tile in pathing
class Node {
  public:
    Node();
    int x, y;
    double gCost, hCost;
    NodeState state;
    Node* bestParent;
};

// Comparator object to sort nodes inside priority queues
struct compareNodePtrs {
  bool operator() (const Node* a, const Node* b) const {
    return a->gCost + a->hCost > b->gCost + b->hCost;
  }
};

class AStar {
  public:
    AStar(Map* map, int startX, int startY, int endX, int endY);
    ~AStar();
    bool calculate();
    std::pair<int, int> walk();
  private:
    int startX, startY, endX, endY;
    Map* map;
    Node* nodes;
    double gCost(Node* node);
    // std::vector<std::pair<int, int>> backtrackPath(Node* node);
    // std::vector<std::pair<int, int>> path;
    std::pair<int, int> walk(Node* node, Node* child);
    std::vector<Node*> getNeighbors(Node* node);
    std::priority_queue<Node*, std::vector<Node*>, compareNodePtrs> openQueue;
    // std::priority_queue<Node*, std::vector<Node*>, compareNodePtrs> closedQueue;
};
