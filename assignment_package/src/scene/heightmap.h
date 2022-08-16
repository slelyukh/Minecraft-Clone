#pragma once
#include "chunk.h"
#include <iostream>

enum Biome : unsigned char {
    ICE_SPIKES, SWAMP, ISLAND, TUNDRA, GRASSLAND, VOLCANO,
    SHITLAND, DESERT, DESERT_MOUNTAIN
};
enum BlockType: unsigned char;

class HeightMap
{
private:
    static float grassHeight(float, float);
    static float mountainHeight(float, float);
    static float spikeHeight(float, float);
public:
    HeightMap();
    static std::pair<int, Biome> getHeight(int, int);
    static boolean hasCactus(int, int);
    static int boulderHeight(int, int);
    static float random1(float, float);
    static BlockType getDepth(int, int, int);
};
