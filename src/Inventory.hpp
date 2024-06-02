#pragma once

struct Inventory {
  int wood;
  int CEREAL_SEED;
  int cerealGrain;
  int tinOre;
  int copperOre;
  int slate; // Right now, all "rock"
  int flour;
  int bronze;
  int bread; // TODO - implement
  int morale;
  int points;
  Inventory() : wood(0), CEREAL_SEED(5), cerealGrain(0), tinOre(0), copperOre(0), slate(0), flour(5), bronze(5), bread(0), morale(0), points(5) {};
  // TODO - function to add or subtract counts by Material
};
