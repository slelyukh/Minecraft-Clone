#include "blocktypeworker.h"
using namespace std;
unordered_map<Biome, BlockType, hash<unsigned char>> BlockTypeWorker::m_topBlockMap = {
    {ICE_SPIKES, SNOW},
    {SWAMP, GRASS},
    {ISLAND, GRASS},
    {TUNDRA, SNOWGRASS},
    {GRASSLAND, GRASS},
    {VOLCANO, STONE},
    {SHITLAND, SHIT},
    {DESERT, SAND},
    {DESERT_MOUNTAIN, SANDSTONE},
};
unordered_map<Biome, BlockType, hash<unsigned char>> BlockTypeWorker::m_fillBlockMap = {
    {ICE_SPIKES, SNOW},
    {SWAMP, DIRT},
    {ISLAND, DIRT},
    {TUNDRA, DIRT},
    {GRASSLAND, DIRT},
    {VOLCANO, STONE},
    {SHITLAND, STONE},
    {DESERT, SAND},
    {DESERT_MOUNTAIN, SAND},
};


BlockTypeWorker::BlockTypeWorker(Terrain* t, std::unordered_set<int64_t> &genTer, QMutex* tm,
                                 std::unordered_set<Chunk*> &chunks,
                                 QMutex* bm, glm::ivec2 pos)
 : m_terrain(t), m_chunks(chunks), blockMux(bm), m_tZones(genTer),
   tZonesMux(tm), m_pos(pos)
{}

void BlockTypeWorker::run() {
    int x_pos = m_pos[0];
    int z_pos = m_pos[1];

    for (int x = x_pos; x < x_pos + 64; x += 16) {
        for (int z = z_pos; z < z_pos + 64; z += 16) {
            //load up each chunk
            uPtr<Chunk>& m_chunk = m_terrain->getChunkAt(x,z);
            for(int zl = 0; zl < 16; ++zl) {
                for(int xl = 0; xl < 16; ++xl) {
                    m_chunk->setBlockAt(xl,0,zl, BEDROCK);
                }
            }
            //initialize stone separately to allow more efficient memory
            // caching
            for(int zl = 0; zl < 16; ++zl) {
                for(int yl = 1; yl < 128; ++yl) {
                    for(int xl = 0; xl < 16; ++xl) {
                        //FOR CAVES
                        //BlockType b = HeightMap::getDepth(x+xl,yl,z+zl);
                        //m_chunk->setBlockAt(xl,yl,zl, b);

                        //FOR NO CAVES
                        m_chunk->setBlockAt(xl,yl,zl, STONE);
                    }

                }
            }
            boolean superChunk = HeightMap::random1(x, z) < (1.f/200.f);
            for(int xl = 0; xl < 16; ++xl) {
                for(int zl = 0; zl < 16; ++zl) {
                    std::pair<int, Biome> ground = HeightMap::getHeight(x + xl, z + zl);
                    int yMax = ground.first;
                    yMax = glm::clamp(0, 254, yMax);
                    Biome b = ground.second;
                    if (b == VOLCANO) {
                        if (yMax < 138) {
                            for (int y = yMax; y <= 138; y++) {
                                m_chunk->setBlockAt(xl, y, zl, WATER);
                            }
                            for (int y = 128; y < yMax; y++) {
                                m_chunk->setBlockAt(xl, y, zl, DIRT);
                            }
                        } else if (yMax < 144) {
                            for (int y = 128; y < yMax; y++) {
                                m_chunk->setBlockAt(xl, y, zl, DIRT);
                            }
                            m_chunk->setBlockAt(xl, yMax, zl, GRASS);
                        } else if (yMax > 163) {
                            for (int y = 128; y < 150; y++) {
                                m_chunk->setBlockAt(xl, y, zl, LAVA);
                            }

                        } else {
                            for (int y = 128; y < yMax; y++) {
                                m_chunk->setBlockAt(xl, y, zl, m_fillBlockMap.at(b));
                            }
                            if (HeightMap::boulderHeight(x + xl, z + zl) > 0) {
                                m_chunk->setBlockAt(xl, yMax, zl, OBSIDIAN);
                            } else {
                                m_chunk->setBlockAt(xl, yMax, zl, m_topBlockMap.at(b));
                            }
                        }
                        continue;
                    }
                    m_chunk->setBlockAt(xl, yMax, zl, m_topBlockMap.at(b));
                    if(b == ICE_SPIKES || b == DESERT_MOUNTAIN) {
                        for (int y = 128; y < yMax && y < 145; y++) {
                            m_chunk->setBlockAt(xl, y, zl, m_fillBlockMap.at(b));
                        }
                        m_chunk->setBlockAt(xl, yMax, zl, m_fillBlockMap.at(b));
                        for (int y = 145; y <= yMax; y++) {
                            m_chunk->setBlockAt(xl, y, zl, m_topBlockMap.at(b));
                        }
                    } else {
                        for (int y = 128; y < yMax; y++) {
                            m_chunk->setBlockAt(xl, y, zl, m_fillBlockMap.at(b));
                        }
                    }

                    if (b == ISLAND && yMax < 140) {
                        for (int y = 128; y <= yMax; y++) {
                            m_chunk->setBlockAt(xl, y, zl, SAND);
                        }
                    }
                    if (yMax < 138) {
                        for (int y = yMax; y <= 138; y++) {
                            m_chunk->setBlockAt(xl, y, zl, WATER);
                        }
                        if(b == TUNDRA || b == ICE_SPIKES || b == SHITLAND) {
                            m_chunk->setBlockAt(xl, 138, zl, ICE);
                        }
                    } else {
                        if (b == DESERT && !superChunk && HeightMap::hasCactus(x + xl, z + zl)) {
                            m_chunk->setBlockAt(xl, yMax+1, zl, CACTUS);
                            m_chunk->setBlockAt(xl, yMax+2, zl, CACTUS);
                            m_chunk->setBlockAt(xl, yMax+3, zl, CACTUS);
                        }
                        if (b == GRASSLAND && HeightMap::hasCactus(x + xl, z + zl) &&
                                xl > 2 && zl > 2 && xl < 14 && zl < 14
                                && HeightMap::random1(x + xl, z + zl) < 0.2) {
                            for (int i = 0; i < 6; i++) {
                                if (i == 4) {
                                    for (int k = 0; k < 5; k++) {
                                        for (int j = 0; j < 5; j++) {
                                            m_chunk->setBlockAt(xl - 2 + k, yMax+i+1, zl - 2+j, LEAVES);
                                        }
                                    }
                                }
                                if (i == 5) {
                                    for (int k = 0; k < 3; k++) {
                                        for (int j = 0; j < 3; j++) {
                                            m_chunk->setBlockAt(xl - 1 + k, yMax+i+1, zl -1+j, LEAVES);
                                        }
                                    }
                                }
                                m_chunk->setBlockAt(xl, yMax+i+1, zl, WOOD);
                            }
                            m_chunk->setBlockAt(xl, yMax+6+1, zl, LEAVES);

                        }
                    }
                    if (b == SHITLAND){
                        for (int i = 0; i < HeightMap::boulderHeight(x + xl, z + zl); i++) {
                            m_chunk->setBlockAt(xl, yMax+i+1, zl, COBBLE);
                        }
                    }
                }
            }
            std::pair<int, Biome> seed = HeightMap::getHeight(x + 8, z + 8);
            int seedHeight = seed.first + 1;
            Biome seedBiome = seed.second;
            if (superChunk) {
                if (seedBiome == DESERT) {
                    //pyramid
                    for(int zl = 0; zl < 15; ++zl) {
                            for(int xl = 0; xl < 15; ++xl) {
                                int layer = glm::min(glm::min(zl,xl), glm::min(14 - zl, 14 - xl)) + 1;
                                for (int y = 0; y < layer; y++) {
                                    m_chunk->setBlockAt(xl, y + seedHeight, zl, SANDSTONE);
                                }
                            }
                    }
                }
                if (seedBiome == TUNDRA) {
                    //igloo
                    std::array<int, 40> layer0x = {7, 8, 9,
                                               7, 8, 9,
                                               6, 7, 8, 9, 10,
                                              5,6,7,8,9,10,11,
                                              5,6,7,8,9,10,11,
                                              5,6,7,8,9,10,11,
                                              6,7,8,9,10,
                                              7,8,9,};
                    std::array<int, 17> layer1and2x = {7, 9,
                                               7, 9,
                                               6, 10,
                                              5,11,
                                              5,11,
                                              5,11,
                                              6,10,
                                              7,8,9};
                    std::array<int, 23> layer3x = {8,
                                               8,
                                               7, 8, 9,
                                              6,7,8,9,10,
                                              6,7,8,9,10,
                                              6,7,8,9,10,
                                              7,8,9,};
                    std::array<int, 9> layer4x = {7,8,9,
                                              7,8,9,
                                              7,8,9,};
                    std::array<int, 40> layer0z = {7,7,7,
                                               8,8,8,
                                               9,9,9,9,9,
                                              10,10,10,10,10,10,10,
                                              11,11,11,11,11,11,11,
                                              12,12,12,12,12,12,12,
                                              13,13,13,13,13,
                                              14,14,14,};
                    std::array<int, 17> layer1and2z = {7, 7,
                                               8, 8,
                                               9, 9,
                                              10,10,
                                              11,11,
                                              12,12,
                                              13,13,
                                              14,14,14};
                    std::array<int, 23> layer3z = {7,
                                               8,
                                               9, 9, 9,
                                              10,10,10,10,10,
                                              11,11,11,11,11,
                                              12,12,12,12,12,
                                              13,13,13,};
                    std::array<int, 9> layer4z = {10,10,10,
                                              11,11,11,
                                              12,12,12,};
                    for (int i = 0; i < 40; i++) {
                        m_chunk->setBlockAt(layer0x[i], seedHeight - 1, layer0z[i], SNOW);
                    }
                    for (int j = 0; j < 2; j++) {
                        for (int i = 0; i < 17; i++) {
                            m_chunk->setBlockAt(layer1and2x[i], seedHeight + j, layer1and2z[i], SNOW);
                        }

                    }
                    for (int i = 0; i < 23; i++) {
                        m_chunk->setBlockAt(layer3x[i], seedHeight+2, layer3z[i], SNOW);
                    }
                    for (int i = 0; i < 9; i++) {
                        m_chunk->setBlockAt(layer4x[i], seedHeight+3, layer4z[i], SNOW);
                    }
                }
                if (seedBiome == SHITLAND) {
                    //igloo
                    std::array<int, 16> layer1to4x = {5,6,10,11,
                                                      5,6,10,11,
                                                      5,6,10,11,
                                                      5,6,10,11};
                    std::array<int, 14> layer5x = {6,7,8,9,10,
                                                   6,10,
                                                   6,10,
                                                   6,7,8,9,10,};
                    std::array<int, 16> layer1to4z = {8,8,8,8,
                                                      9,9,9,9,
                                                      12,12,12,12,
                                                      13,13,13,13};
                    std::array<int, 14> layer5z = {9,9,9,9,9,
                                                   10,10,
                                                   11,11,
                                                   12,12,12,12};
                    for (int j = 0; j < 4; j++) {
                        for (int i = 0; i < 16; i++) {
                            m_chunk->setBlockAt(layer1to4x[i], seedHeight, layer1to4z[i], STONE);
                        }

                    }
                    for (int i = 0; i < 14; i++) {
                        m_chunk->setBlockAt(layer5x[i], seedHeight + 4, layer5z[i], OBSIDIAN);
                    }
                }
            }
            m_chunk->m_blocksFilled = true;

            // let generated terrain know chunk block data is generated.
            blockMux->lock();
            m_chunks.insert(m_chunk.get());
            blockMux->unlock();
        }
    }
    tZonesMux->lock();
    m_tZones.insert(Terrain::toKey(x_pos, z_pos));
    tZonesMux->unlock();
}
