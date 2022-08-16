For procedural assets I added Cactuses and pyramids in the desert, Trees in the grassland,
igloos in the tundra, boulders in shitland and obsidian circles in volcanos.

Cactuses were placed using worley noise with worley noise sampled to have points very often
in the terrain. Cactuses were only placed if minDist in the worley noise was very close to a block
corner to prevent multiple cactuses from being placed around the same worley noise point.

Trees were placed using the same strategy as cactuses except with more spread out worley noise
points and a random number generator that made it so that only 20% of trees that could be
made were made. Tree generation also prevented trees from being generated too close to the edge of
chunks. This was fine because trees are very sparse in the grassland so this part of their
generation wouldn't be obvious.

Pyramids and igloos were placed uniformly at random by using a random function for each chunk
that designates every 1 in 200 chunks as a "super chunk". Every pyramid is 15 by 15 blocks
and starts at the origin of it's chunk. To create igloos I made an igloo in minecraft and wrote
down all the x and z coordinates of blocks and created an igloo on every super chunk in the tundra.


Boulders and obsidian circles used the same function called getBoulderHeight. getBoulderHeight used
worley noise with height being 0 in most places but between 1 and 3 depending on how close a block was to a
worley noise point. This allowed for some variety in boulder shapes.
Obsidian circles ignored boulder height and just changed the top block to obsidian if the returned 
boulder height was above 0.