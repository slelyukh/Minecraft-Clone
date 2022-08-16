#include "heightmap.h"

using namespace glm;

float HeightMap::random1( float x, float z ) {
    vec2 p = vec2(x,z);
    return fract(sin(vec2(dot(p, vec2(154.3, 281.6)),
                 dot(p, vec2(235.7,149.1))))
                 * 63242.2581f).x;
}

vec2 random2( vec2 p ) {
    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)),
                 dot(p, vec2(269.5,183.3))))
                 * 43758.5453f);
}

vec3 random3(vec3 p) {
    return fract(sin(vec3(dot(p,vec3(127.1, 311.7, 412.6)),
                          dot(p,vec3(269.5, 183.3, 789.2)),
                          dot(p,vec3(420.6, 631.2, 345.2))
                    )) * 43758.5453f);
}

float surflet(vec2 P, vec2 gridPoint) {
    // Compute falloff function by converting linear distance to a polynomial
    float distX = abs(P.x - gridPoint.x);
    float distY = abs(P.y - gridPoint.y);
    float tX = 1 - 6 * pow(distX, 5.f) + 15 * pow(distX, 4.f) - 10 * pow(distX, 3.f);
    float tY = 1 - 6 * pow(distY, 5.f) + 15 * pow(distY, 4.f) - 10 * pow(distY, 3.f);
    // Get the random vector for the grid point
    vec2 gradient = 2.f * random2(gridPoint) - vec2(1.f);
    // Get the vector from the grid point to P
    vec2 diff = P - gridPoint;
    // Get the value of our height field by dotting grid->P with our gradient
    float height = dot(diff, gradient);
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * tX * tY;
}

float WorleyNoise(float x, float y) {
    vec2 uv = vec2(x,y);
    uv *= 8.0; // Now the space is 10x10 instead of 1x1. Change this to any number you want.
    vec2 uvInt = floor(uv);
    vec2 uvFract = fract(uv);
    float minDist = 1.0; // Minimum distance initialized to max.
    for(int y = -1; y <= 1; ++y) {
        for(int x = -1; x <= 1; ++x) {
            vec2 neighbor = vec2(float(x), float(y)); // Direction in which neighbor cell lies
            vec2 point = random2(uvInt + neighbor); // Get the Voronoi centerpoint for the neighboring cell
            vec2 diff = neighbor + point - uvFract; // Distance between fragment coord and neighbor’s Voronoi point
            float dist = length(diff);
            minDist = glm::min(minDist, dist);
        }
    }
    return minDist;
}

float perlinNoise(float x, float y) {
        float surfletSum = 0.f;
        x = x * 15.f;
        y = y * 15.f;
        vec2 uv = vec2(x,y);
        // Iterate over the four integer corners surrounding uv
        for(int dx = 0; dx <= 1; ++dx) {
                for(int dy = 0; dy <= 1; ++dy) {
                        surfletSum += surflet(uv, floor(uv) + vec2(dx, dy));
                }
        }
        // abs for lines and shet
        return surfletSum;
}

float surflet3d(vec3 p, vec3 gridPoint) {
   // Compute the distance between p and the grid point along each axis, and warp it with a
   // quintic function so we can smooth our cells
   vec3 t2 = abs(p - gridPoint);
   vec3 t = vec3(1.f) - 6.f * pow(t2.x, 5.f) + 15.f * pow(t2.y, 4.f) - 10.f * pow(t2.z, 3.f);
   // Get the random vector for the grid point (assume we wrote a function random2
   // that returns a vec2 in the range [0, 1])
   vec3 gradient = random3(gridPoint) * 2.f - vec3(1., 1., 1.);
   // Get the vector from the grid point to P
   vec3 diff = p - gridPoint;
   // Get the value of our height field by dotting grid->P with our gradient
   float height = dot(diff, gradient);
   // Scale our height field (i.e. reduce it) by our polynomial falloff function
   return height * t.x * t.y * t.z;
}

float perlinNoise3d(float x, float y, float z) {
    float surfletSum = 0.f;
    vec3 uv = vec3(x,y,z);
    for(int dx = 0; dx <= 1; ++dx) {
        for(int dy = 0; dy <= 1; ++dy) {
            for(int dz = 0; dz <= 1; ++dz) {
                surfletSum += surflet3d(uv, floor(uv) + vec3(dx, dy, dz));
            }
        }
    }
    return surfletSum;
}

HeightMap::HeightMap()
{

}

int HeightMap::boulderHeight(int xC, int zC) {
    float noise = WorleyNoise(xC/200.f, zC/200.f);
    if (noise < 0.035) {
        return 3;
    } else if (noise < 0.08) {
        return 2;
    } else if (noise < 0.12) {
        return 1;
    }
    return 0;
}

boolean HeightMap::hasCactus(int xC, int zC) {
    return WorleyNoise(xC/2.f, zC/2.f) < 0.045;
}

float HeightMap::grassHeight(float x, float y) {
    float val = 1.f + perlinNoise(x * 8, y * 8);
    val = pow(val, 0.2f);
    val -= 0.5f;
    val = glm::clamp(val, 0.f, 1.f);
    val += (1 - WorleyNoise(x*2,y*2)) * 0.8;
    return val;
}

float HeightMap::mountainHeight(float x, float y) {
    float val = 0.8f + perlinNoise(x * 2.5, y * 2.5);
    val = pow(val, 3.f);
    val -= 0.4;
    return glm::clamp(val, 0.f, 1.f);
}

float HeightMap::spikeHeight(float x, float y) {
    float val = 0.8f + perlinNoise(x, y);
    val = pow(val, 3.f);
    val -= 0.4;
    return val;
}


std::pair<int, Biome> HeightMap::getHeight(int xC, int zC) {
    float x = (float) xC / 2700.f;
    float z = (float) zC / 2700.f;
    //TODO: move random calculations to if statements so we only calculate 4 heights
    // instead of 9
    float biomeX = x/1.5f;
    float biomeZ = z/1.5f;
    float temp = perlinNoise(biomeX, biomeZ);
    temp += 0.5;
    temp = 0.5*(smoothstep(0.3f, 0.4f, temp) + smoothstep(0.58f, 0.68f, temp));
    float stdGrass = grassHeight(x,z);
    float wet = WorleyNoise(biomeX, biomeZ)*0.7 + WorleyNoise(biomeX*2, biomeZ*2)*0.3;
    wet = 0.5*(smoothstep(0.27f, 0.38f, wet) + smoothstep(0.58f, 0.68f, wet));
//    float iceSpikes = pow(spikeHeight(x, z) * 1.8, 4) + 11;
//    float swamp = 12;
//    float island = (1 - WorleyNoise(x*5, z*5)) * 14 + perlinNoise(x*4, z*4) * 8;
//    float tundra = stdGrass * 19;
//    float grassland = stdGrass * 16;
//    float volcano = pow((1 - WorleyNoise(x*4, z*4)), 1.5) * 10 + perlinNoise(x*5, z*5)*2;
//    volcano = pow(volcano, 1.6f) + 5;
//    float shitland = stdGrass * 10 + 2;
//    float desert = stdGrass * 10 + 4;
//    float desertMountain = mountainHeight(x, z) * 44 + 6;
    float h1, h2;
    int height;
    if (wet < 0.5) {
        // BOTTOM LEFT
        if (temp < 0.5) {
            float shitland = grassHeight(x+40,z+90)*3 + stdGrass * 7 + 2;
            float desert = stdGrass * 10 + 4;
            float tundra = stdGrass * 19;
            float grassland = stdGrass * 16;
            h1 = mix(shitland, desert, temp * 2);
            h2 = mix(tundra, grassland, temp * 2);
            height = 128 + mix(h1, h2, wet*2);
        }
        // BOTTOM RIGHT
        else {
            float desert = stdGrass * 10 + 4;
            float desertMountain = mountainHeight(x, z) * 44 + 6;
            float grassland = stdGrass * 16;
            float volcano = pow((1 - WorleyNoise(x*4, z*4)), 1.5) * 10 + perlinNoise(x*5, z*5)*2;
            volcano = pow(volcano, 1.6f) + 5;
            h1 = mix(desert, desertMountain, (temp-0.499999) * 2);
            h2 = mix(grassland, volcano, (temp-0.499999) * 2);
            height = 128 + mix(h1, h2, wet*2);
        }

    } else {
        // TOP LEFT
        if (temp < 0.5) {
            float tundra = stdGrass * 19;
            float grassland = stdGrass * 16;
            float iceSpikes = pow(spikeHeight(x, z) * 1.8, 4) + 11;
            float swamp = grassHeight(x*10, z*10)*3 + 7;
            h1 = mix(tundra, grassland, temp * 2);
            h2 = mix(iceSpikes, swamp, temp * 2);
            height = 128 + mix(h1, h2, (wet-0.499999)*2);
        }
        // TOP RIGHT
        else {
            float grassland = stdGrass * 16;
            float volcano = pow((1 - WorleyNoise(x*4, z*4)), 1.5) * 10 + perlinNoise(x*5, z*5)*2;
            volcano = pow(volcano, 1.6f) + 5;
            float swamp = grassHeight(x*10, z*10)*3 + 7;
            float island = (1 - WorleyNoise(x*5, z*5)) * 14 + perlinNoise(x*4, z*4) * 8;
            h1 = mix(grassland, volcano, (temp-0.499999) * 2);
            h2 = mix(swamp, island, (temp-0.499999) * 2);
            height = 128 + mix(h1, h2, (wet-0.499999)*2);
        }
    }
    Biome biomeType;
        if (temp < 0.33) {
            if (wet < 0.33) {
                biomeType = SHITLAND;
            } else if (wet < 0.66) {
                biomeType = TUNDRA;
            } else {
                biomeType = ICE_SPIKES;
            }
        } else if (temp < 0.66) {
            if (wet < 0.33) {
                biomeType = DESERT;
            } else if (wet < 0.66) {
                biomeType = GRASSLAND;
            } else {
                biomeType = SWAMP;
            }
        } else {
            if (wet < 0.33) {
                biomeType = DESERT_MOUNTAIN;
            } else if (wet < 0.66) {
                biomeType = VOLCANO;
            } else {
                biomeType = ISLAND;
            }
        }
    return std::pair(height, biomeType);
}

BlockType HeightMap::getDepth(int x, int y, int z) {
    float perlin = perlinNoise3d(x/15.f,y/15.f,z/15.f);
    //std::cout << "Perlin Noise: " << perlin << std::endl;
    if (perlin > 0.f) {
        return STONE;
    } else {
        if (y < 25) {
            return LAVA;
        }
        return EMPTY;
    }
}
