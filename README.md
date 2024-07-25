### Derinkuyu

A rudimentary base-building game demo made for a C++ independent school project with [Libtcod](https://github.com/libtcod/libtcod). The gameplay aims to be a super bare-bones imitation of Oxygen Not Included and Dwarf Fortress, and the name comes from the the Derinkuyu underground city site in Turkiye.

#### Keybinds / How to Play

The goal is to raise the "morale" of your three villagers ('@' characters) by building them basic amenities like shelter, beds, and a stockpile of bread. After morale passes 6 (all three have bread and a bed) the object shifts to selling as much bronze, flour, and gems to the trader for points before the timer runs out.

WASD - move cursor

Space - move villager to tile; rapidly use on multiple tiles to force everyone to move

Backspace - erase queued jobs; use this if something appears to be buggy

Q - chop trees

E (click once, designate a rectangle with WASD, and click E again to confirm) - dig tiles

I / O / P - plow soil, plant grain, and harvest plants, respectively; uses the same double press rectangle designation as digging

R - use smelter (copper (x2) and tin (x1) to bronze), millstone (grain to flour), or oven (flour to bread)

Z / X / C - Z and C scroll the build menu at the bottom of the screen and X orders construction of that item on the cursor's tile. Buildings require wood and oven / millstone / smelter need stone. When hovering over the trader ('&' character) with the cursor, these three keys instead navigate the trading menu.

#### To-Do
- In-game help sheet with keybinds and game mechanics
- Irrigation systems and multiple cultivable plants
- Changing weather
- Different biomes, both on surface (flat land, cliffs, rock chimneys) and underground (groundwater, caverns, lava)
- Animals and wildlife
- Make chopping / building non-instantaneous
- Defenses and enemy raids
- Add dynamic "lighting" and "fog of war" so whole map isn't always visible
- Replace global inventory with tracking of actual item objects that can be moved and stored on particular tiles
