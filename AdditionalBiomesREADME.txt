This ReadMe describes the process I used to randomly generate my 9 biomes,
which were: Grassland, Mountain, Desert Mountain, Desert, Swamp, Island, Tundra, Ice Spikes and Badlands

I created 9 biomes by randomly generating maps for temperature and moisture
with 3 levels of moisture and 3 levels of temperature. I used perlin noise for temperature and 
Worley noise sampled at two levels and summed for moisture. To create a double smoothstep function
I added two smoothstep functions together to make sure I have 9 distinct biomes from the combination
of two double smoothstep functions.

To interpolate between biomes, I first split the moisture and temperature array
into one of four quadrants of 4 biomes based on whether temperature and moisture were
above or below 0.5. After that, I interpolated heights between biomes in the
same way as described in lecture for a 2 by 2 biome map.

To implement many of the biomes I reused a perlin noise based function I had optimized
for grassland generation with slightly different parameters for different biomes to have
different ranges of terrain height. To implement my Volcanos and island biomes,
I used Worley noise. For volcanos I raised worley noise to a higher power and then
cut off the volcano height at 170 and made there be a hole with lava in it for any
volcano biome height above 170. I also added perlin noise with a small influence to make
the volcanos less perfectly circular. To create swamps, I used Perlin noise in the grassHeight
function sampled at x and z * 10 with only a 3 block range to create highly variable relatively
flat terrain with many small water holes. 