#pragma once
#include <QRunnable>
#include <QMutex>
#include <unordered_set>
#include "scene/terrain.h"
using namespace std;

class Chunk;
class Terrain;
enum BlockType: unsigned char;
enum Biome: unsigned char;
class BlockTypeWorker : public QRunnable {
private:
    Terrain* m_terrain;
    std::unordered_set<Chunk*>& m_chunks;
    QMutex *blockMux;
    std::unordered_set<int64_t>& m_tZones;
    QMutex *tZonesMux;
    glm::ivec2 m_pos;
    static unordered_map<Biome, BlockType, hash<unsigned char>> m_fillBlockMap;

public:
    static unordered_map<Biome, BlockType, hash<unsigned char>> m_topBlockMap;
    BlockTypeWorker(Terrain*, std::unordered_set<int64_t> &, QMutex*,
                    std::unordered_set<Chunk*> &, QMutex*, glm::ivec2);
    void run() override;
};
