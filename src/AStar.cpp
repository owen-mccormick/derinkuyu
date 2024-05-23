#include <vector>
#include <queue>
#include <iostream>
#include <cassert>
#include "math.h"
#include "limits.h"
#include "AStar.hpp"

Node::Node() : x(0), y(0), hCost(INT_MAX), gCost(INT_MAX), bestParent(nullptr), state(NodeState::HIDDEN) {}

AStar::AStar(Map* map, int startX, int startY, int endX, int endY) : map(map), startX(startX), endX(endX), endY(endY) {
  nodes = new Node[map->width * map->height];

  // Assign nodes their coordinates and distances from end
  for (int i = 0; i < map->width; i++) {
    for (int j = 0; j < map->height; j++) {
      nodes[i + map->width * j].x = i;
      nodes[i + map->width * j].y = j;
      nodes[i + map->width * j].hCost = hypot(endX - i, endY - j);
      nodes[i + map->width * j].state = NodeState::HIDDEN;
    }
  }

  openQueue.push(&nodes[startX + map->width * startY]);
  // std::cout << "Queue length: " << openQueue.size() << std::endl;
  nodes[startX + map->width * startY].state = NodeState::OPEN;
  nodes[startX + map->width * startY].gCost = 0;
}

AStar::~AStar() {
  delete [] nodes;
}

// Recursively trace back to the start
int AStar::gCost(Node* node) {
  return node->bestParent == nullptr ? 0 : hypot(node->x - node->bestParent->x, node->y - node->bestParent->y) + gCost(node->bestParent);
}

// See https://gamedev.stackexchange.com/questions/98090/pathfinding-in-a-2d-sidescrolling-strategy-game
std::vector<Node*> AStar::getNeighbors(Node* node) {
  std::vector<Node*> result;
  int dx[2] = {-1, 1};
  int dy[3] = {-1, 0};

  // Nodes in the air are linked only to nodes below themselves unless they are ladders
  if (map->isActorWalkable(node->x, node->y + 1) && !map->getMaterial(node->x, node->y).climbable) {
    // std::cout << "Detected fall neighbor (" << node->x << ", " << node->y + 1 << ") of node (" << node->x << ", " << node->y << ")\n";
    result.push_back(&nodes[node->x + (node->y + 1) * map->width]);
  } else {
    // Cycle through dx and dy combination neighbors
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++) {
        if (map->isActorWalkable(node->x + dx[i], node->y + dy[j])) {
          // std::cout << "Detected neighbor (" << node->x + dx[i] << ", " << node->y + dy[j] << ") of node (" << node->x << ", " << node->y << ")\n";
          result.push_back(&nodes[node->x + dx[i] + map->width * (node->y + dy[j])]);
        }
      }
    }
    // Ladders are accessible from below and above
    if (map->isActorWalkable(node->x, node->y - 1) && map->getMaterial(node->x, node->y - 1).climbable) {
      result.push_back(&nodes[node->x + map->width * (node->y - 1)]);
    }
    if (map->isActorWalkable(node->x, node->y + 1) && map->getMaterial(node->x, node->y + 1).climbable) {
      result.push_back(&nodes[node->x + map->width * (node->y + 1)]);
    }
  }
  return result;
}

// Returns whether or not a path is found.
bool AStar::calculate() {
  do {
    if (openQueue.size() == 0) {
      // If the queue is empty, there's no possible path
      // std::cout << "No path found" << std::endl;
      return false;
    }
    Node* current = openQueue.top();
    // std::cout << "Examining node (" << current->x << ", " << current->y << ")\n";
    openQueue.pop();
    // std::cout << "Queue len: " << openQueue.size() << std::endl;
    current->state = NodeState::CLOSED;

    if (current->x == endX && current->y == endY) {
      // std::cout << "Found goal" << std::endl;
      return true;
    }

    for (Node* neighbor : getNeighbors(current)) {
      if (neighbor->state != NodeState::CLOSED) {
        if (neighbor->state != NodeState::OPEN || gCost(neighbor) > hypot(neighbor->x - current->x, neighbor->y - current->y) + gCost(current)) {
          // We have a new path and associated cost to record
          neighbor->bestParent = current;
          neighbor->gCost = gCost(neighbor);
          if (neighbor->state != NodeState::OPEN) {
            neighbor->state = NodeState::OPEN;
            // std::cout << "Pushing to neighbors node (" << neighbor->x << ", " << neighbor->y << ")\n";
            openQueue.push(neighbor);
          }
        }
      }
    }
  } while (true);
}

// Return one coordinate pair at a time from the path each time `walk` is called, starting at the start point.
std::pair<int, int> AStar::walk() {
  return walk(&nodes[endX + endY * map->width], nullptr);
}

std::pair<int, int> AStar::walk(Node* node, Node* child) {
  if (node->bestParent != nullptr) {
    if (node->bestParent->bestParent == nullptr) {
      if (child != nullptr) child->bestParent = nullptr;
      return std::pair<int, int>(node->x, node->y);
    } else {
      return walk(node->bestParent, node);
    }
  } else {
    return std::pair<int, int>(node->x, node->y);
  }
}
