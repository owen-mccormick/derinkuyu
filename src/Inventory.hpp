#pragma once

struct Inventory {
  int wood;
  int cerealSeed;
  int cerealGrain;
  int tinOre;
  int copperOre;
  int slate; // Right now, all "rock"
  int flour;
  int bronze;
  int bread; // TODO - implement
  int moraleBed;
  int moraleFood;
  int points;
  int gems;
  Inventory() : wood(0), cerealSeed(5), cerealGrain(0), tinOre(0), copperOre(0), slate(0), flour(0), bronze(0), bread(0), moraleBed(0), moraleFood(0), points(0), gems(0) {};
  // TODO - function to add or subtract counts by Material
};
