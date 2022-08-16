#include "terrain.h"
#include <stdexcept>
#include <iostream>

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), m_generatedTerrain(), mp_context(context)
{}

Terrain::~Terrain() {
    for (auto& chunk : m_chunks) {
        Chunk* c = chunk.second.get();
        if(c->mcr_VBOcreated) {
            c->destroy();
        }
    }
}

// Combine two 32-bit ints into one 64-bit int
// where the upper 32 bits are X and the lower 32 bits are Z
int64_t Terrain::toKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    int64_t x64 = x;
    int64_t z64 = z;

    // Set all lower 32 bits to 1 so we can & with Z later
    xz = (xz & (x64 << 32)) | 0x00000000ffffffff;

    // Set all upper 32 bits to 1 so we can & with XZ
    z64 = z64 | 0xffffffff00000000;

    // Combine
    xz = xz & z64;
    return xz;
}

glm::ivec2 toCoords(int64_t k) {
    // Z is lower 32 bits
    int64_t z = k & 0x00000000ffffffff;
    // If the most significant bit of Z is 1, then it's a negative number
    // so we have to set all the upper 32 bits to 1.
    // Note the 8    V
    if(z & 0x0000000080000000) {
        z = z | 0xffffffff00000000;
    }
    int64_t x = (k >> 32);

    return glm::ivec2(x, z);
}

// Surround calls to this with try-catch if you don't know whether
// the coordinates at x, y, z have a corresponding Chunk
BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    if(hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if(y < 0 || y >= 256) {
            return EMPTY;
        }
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        return c->getBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                             static_cast<unsigned int>(y),
                             static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

BlockType Terrain::getBlockAt(glm::vec3 p) const {
    //block rounding
    glm::ivec3 offset = glm::ivec3(0,0,0);
    offset.x = glm::min(0.f,glm::sign(p.x));
    offset.y = glm::min(0.f,glm::sign(p.y));
    offset.z = glm::min(0.f,glm::sign(p.z));
    glm::ivec3 blockpos = glm::ivec3(glm::floor(p)) + offset;
    return getBlockAt(blockpos.x, blockpos.y, blockpos.z);
}

bool Terrain::hasChunkAt(int x, int z) const {
    // Map x and z to their nearest Chunk corner
    // By flooring x and z, then multiplying by 16,
    // we clamp (x, z) to its nearest Chunk-space corner,
    // then scale back to a world-space location.
    // Note that floor() lets us handle negative numbers
    // correctly, as floor(-1 / 16.f) gives us -1, as
    // opposed to (int)(-1 / 16.f) giving us 0 (incorrect!).
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.find(toKey(16 * xFloor, 16 * zFloor)) != m_chunks.end();
}


uPtr<Chunk>& Terrain::getChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks[toKey(16 * xFloor, 16 * zFloor)];
}


const uPtr<Chunk>& Terrain::getChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.at(toKey(16 * xFloor, 16 * zFloor));
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        c->setBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                      static_cast<unsigned int>(y),
                      static_cast<unsigned int>(z - chunkOrigin.y),
                      t);
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

Chunk* Terrain::instantiateChunkAt(int x, int z) {
    uPtr<Chunk> chunk = mkU<Chunk>(mp_context);
    Chunk *cPtr = chunk.get();
    m_chunks[toKey(x, z)] = move(chunk);
    // Set the neighbor pointers of itself and its neighbors
    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        cPtr->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        cPtr->linkNeighbor(chunkWest, XNEG);
    }
    return cPtr;
}


void Terrain::draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram) {
    for(int x = minX; x <= maxX; x += 16) {
        for(int z = minZ; z <= maxZ; z += 16) {
            const uPtr<Chunk> &chunk = getChunkAt(x, z);
            if (chunk.get() == nullptr || !chunk->mcr_VBOcreated) {
                continue;
            }
            shaderProgram->setModelMatrix(glm::translate(glm::mat4(1.f), glm::vec3(x, 0.f, z)));
            shaderProgram->drawInterleaved(*chunk);
        }
    }
    for(int x = minX; x <= maxX; x += 16) {
        for(int z = minZ; z <= maxZ; z += 16) {
            const uPtr<Chunk> &chunk = getChunkAt(x, z);
            if (chunk.get() == nullptr || !chunk->mcr_VBOcreated) {
                continue;
            }
            shaderProgram->setModelMatrix(glm::translate(glm::mat4(1.f), glm::vec3(x, 0.f, z)));
            shaderProgram->drawInterleavedTransparent(*chunk);
        }
    }
}

void Terrain::drawOpaque(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram) {
    for(int x = minX; x <= maxX; x += 16) {
        for(int z = minZ; z <= maxZ; z += 16) {
            const uPtr<Chunk> &chunk = getChunkAt(x, z);
            if (chunk.get() == nullptr || !chunk->mcr_VBOcreated) {
                continue;
            }
            shaderProgram->setModelMatrix(glm::translate(glm::mat4(1.f), glm::vec3(x, 0.f, z)));
            shaderProgram->drawInterleaved(*chunk);
        }
    }
}

void Terrain::spawnVBOWorker(Chunk* c) {
    VBOWorker* w = new VBOWorker(c, m_chunksWithVBOData,
                                 &m_chunksWithVBODataMux);
    QThreadPool::globalInstance()->start(w);
}

void Terrain::spawnBlockTypeWorker(int64_t id) {
    ivec2 coords = toCoords(id);
    for (int x = coords.x; x < coords.x + 64; x += 16) {
        for (int z = coords.y; z < coords.y + 64; z += 16) {
            uPtr<Chunk> chunk = mkU<Chunk>(mp_context);
            instantiateChunkAt(x,z);
        }
    }
    BlockTypeWorker* w = new BlockTypeWorker(this, m_generatedTerrain, &m_genTerrainMux,
                                             m_chunksWithBlockData, &m_chunksWithBlockDataMux,
                                             ivec2(coords.x, coords.y));
    QThreadPool::globalInstance()->start(w);
}

void Terrain::spawnVBOWorkers(const std::unordered_set<Chunk*> &chunks) {
    for (Chunk *c : chunks) {
        spawnVBOWorker(c);
    }
}

void Terrain::spawnBTWorkers(const std::unordered_set<int64_t> &ids) {
    for (int64_t id : ids) {
        spawnBlockTypeWorker(id);
    }
}

void Terrain::checkThreadResults() {
    m_chunksWithBlockDataMux.lock();
    spawnVBOWorkers(m_chunksWithBlockData);
    m_chunksWithBlockData.clear();
    m_chunksWithBlockDataMux.unlock();

    m_chunksWithVBODataMux.lock();
    for (uPtr<ChunkVBOData>& c : m_chunksWithVBOData) {
        c->mp_chunk->createVBOdata(c->m_idxDataOpaque, c->m_vboDataOpaque);
        c->mp_chunk->createVBOdataTransparent(c->m_idxDataTransparent, c->m_vboDataTransparent);
    }
    m_chunksWithVBOData.clear();
    m_chunksWithVBODataMux.unlock();
}

QSet<int64_t> Terrain::zonesBorderingZone(ivec2 zonePos, int radius) {
    QSet<int64_t> zones;
    int inc = radius * 64;
    for (int x = zonePos[0] - inc; x <= zonePos[0] + inc; x += 64) {
        for (int z = zonePos[1] - inc; z <= zonePos[1] + inc; z += 64) {
            zones.insert(toKey(x, z));
        }
    }
    return zones;
}

boolean Terrain::terrainZoneExists(int64_t id) {
    m_genTerrainMux.lock();
    boolean ret = m_generatedTerrain.find(id) != m_generatedTerrain.end();
    m_genTerrainMux.unlock();
    return ret;
}

void Terrain::tryExpansion(vec3 pos, vec3 posPrev) {
    ivec2 currZone(floor(pos[0] / 64.f) * 64.f, floor(pos[2] / 64.f) * 64.f);
    ivec2 prevZone(floor(posPrev[0] / 64.f) * 64.f, floor(posPrev[2] / 64.f) * 64.f);

    QSet<int64_t> currBorderingZones = zonesBorderingZone(currZone, 2);
    QSet<int64_t> prevBorderingZones = zonesBorderingZone(prevZone, 2);

    for (auto id : prevBorderingZones) {
        if (!currBorderingZones.contains(id)) {
            ivec2 coord = toCoords(id);
            for (int x = coord.x; x < coord.x + 64; x += 16) {
                for (int z = coord.y; z < coord.y + 64; z += 16) {
                    auto &chunk = getChunkAt(x,z);
                    chunk->destroy();
                }
            }
        }
    }

    for (auto id : currBorderingZones) {
        if (terrainZoneExists(id)) {
            if (!prevBorderingZones.contains(id)) {
                ivec2 coords = toCoords(id);
                for (int x = coords.x; x < coords.x + 64; x += 16) {
                    for (int z = coords.y; z < coords.y + 64; z += 16) {
                        auto &chunk = getChunkAt(x,z);
                        spawnVBOWorker(chunk.get());
                    }
                }
            }
        }
        else {
            spawnBlockTypeWorker(id);
        }
    }
}

void Terrain::multiThreadedWork(glm::vec3 pos, glm::vec3 posPrev, float dT) {
    m_tryExpansionTimer += dT;

    if (m_tryExpansionTimer < 0.5f) {
        return;
    }

    tryExpansion(pos, posPrev);
    checkThreadResults();
    m_tryExpansionTimer = 0.f;
}

void Terrain::createVBOData(Chunk *c) {
    m_chunksWithBlockDataMux.lock();
    spawnVBOWorker(c);
    m_chunksWithBlockData.clear();
    m_chunksWithBlockDataMux.unlock();
    QThreadPool::globalInstance()->waitForDone();

    m_chunksWithVBODataMux.lock();
    for (uPtr<ChunkVBOData>& c : m_chunksWithVBOData) {
        c->mp_chunk->createVBOdata(c->m_idxDataOpaque, c->m_vboDataOpaque);
        c->mp_chunk->createVBOdataTransparent(c->m_idxDataTransparent, c->m_vboDataTransparent);
    }
    m_chunksWithVBOData.clear();
    m_chunksWithVBODataMux.unlock();
}

void Terrain::createRiver(ivec2 pos, float angle) {
    Lsystem river = Lsystem("FF[-F][+F]");
    river.rules.push_back(std::string("FF"));
    river.rules.push_back(std::string("F[-L]"));
    river.rules.push_back(std::string("F[+R]"));
    river.constructLString(3, 0, .2); //iteration, replacement, branch
    std::cout << river.lstring << std::endl;

    std::stack<Position> s;
    Position currPos = Position(vec3(pos.x, 140, pos.y), angle);
    std::map<int64_t, int> chunks;

    ivec2 start = pos;
    ivec2 end = pos;
    for (int i=0; i<river.lstring.length(); i++) {
        char c = river.lstring.at(i);
        switch (c) {
        case 'F': {
            int fsize = 3.f + ((float) rand() / (RAND_MAX)) * 2.f; //5-10 block length per F
            float ftype = ((float) rand() / (RAND_MAX)); //curves?
            float curve = 0.f;
            if (ftype < .25) { //left
                curve = 3.5;
            } else if (ftype >= .25 < .50) { //right
                curve = -3.5;
            }
            for (int j=0; j<3; j++) {
                currPos.rotate(curve);
                end = currPos.move(fsize);
                if (hasChunkAt(start.x, start.y) && hasChunkAt(end.x, end.y)) {
                    std::pair<int, Biome> h1 = HeightMap::getHeight(start.x, start.y);
                    std::pair<int, Biome> h2 = HeightMap::getHeight(end.x, end.y);
                    carveSeg(start, end, 3, 3, h1.first-1, h2.first-1, chunks);
                }

//                if (hasChunkAt(start.x, start.y)) { //inside circle
//                   int64_t end_chunk = toKey(start.x, start.y);
//                   setBlockAt(start.x, 180, start.y, STONE);
//                   if (chunks.count(end_chunk) == 0) {
//                       chunks.insert({end_chunk, 0});
//                   }
//                }
//                if (hasChunkAt(end.x, end.y)) { //inside circle
//                   int64_t end_chunk = toKey(end.x, end.y);
//                   setBlockAt(end.x, 180, end.y, STONE);
//                   if (chunks.count(end_chunk) == 0) {
//                       chunks.insert({end_chunk, 0});
//                   }
//                }
                start = end;
            }
            break;
        }
        case '[': {
            s.push(currPos);
            break;
        }
        case ']': {
            currPos = s.top();
            s.pop();
            break;
        }
        case '+': {
            currPos.rotate(30.f);
            break;
        }
        case '-': {
            currPos.rotate(-30.f);
            break;
        }
        default:
            std::cout << "unknown char" << std::endl;
        }
    }

//    carveSeg(ivec2(45,45), ivec2(90,90), 5, 5, 140, chunks);
//    carveSeg(ivec2(90,90), ivec2(110,110), 5, 5, 140, chunks);
//    carveSeg(ivec2(45,45), ivec2(0,45), 5, 5, 140, chunks);

    for (auto& a : chunks) {
        ivec2 chunkXZ = toCoords(a.first);
        uPtr<Chunk>& c = getChunkAt(chunkXZ.x, chunkXZ.y);
        c->destroy();
        createVBOData(c.get());
    }
}

float roundedCone(vec3 p, vec3 c1, vec3 c2, float r1, float r2) {
    //coppied from https://iquilezles.org/articles/distfunctions/
    // sampling independent computations (only depend on shape)
    vec3  ba = c2 - c1;
    float l2 = dot(ba,ba);
    float rr = r1 - r2;
    float a2 = l2 - rr*rr;
    float il2 = 1.0/l2;

    // sampling dependant computations
    vec3 pa = p - c1;
    float y = dot(pa,ba);
    float z = y - l2;
    float x2 = dot(pa*l2 - ba*y,pa*l2 - ba*y);
    float y2 = y*y*l2;
    float z2 = z*z*l2;

    // single square root!
    float k = sign(rr)*rr*rr*x2;
    if( sign(z)*a2*z2>k ) return  std::sqrt(x2 + z2)        *il2 - r2;
    if( sign(y)*a2*y2<k ) return  std::sqrt(x2 + y2)        *il2 - r1;
                        return (std::sqrt(x2*a2*il2)+y*rr)*il2 - r1;
}

void rasterizeLine(LineSegment l, std::map<int, float>& entry, std::map<int, float>& exit) {
    //rasterizes along z axis
    for (int i=std::min(l.end1.y, l.end2.y); i<std::max(l.end1.y, l.end2.y); i++) {
        float f = 0.f;
        bool b = l.getIntersection(i, &f);
        if (l.dy < 0) { //dy = dz
            entry.insert({i, f});
        } else if (l.dy > 0) {
            exit.insert({i, f});
        } else {
            entry.insert({std::min(l.end1.x, l.end2.x), f});
            exit.insert({std::max(l.end1.x, l.end2.x), f});
        }
    }
}

void Terrain::fillRiver(int z, int x1, int x2, int y, int r1, int r2, vec3 c1, vec3 c2, std::map<int64_t, int>& chunks) {
    int bound = std::max(r1, r2);
    int ymin = y-bound-1;
    int ymax = y+bound+7;
    int xmin = std::min(x1,x2)-1;
    int xmax = std::max(x1,x2)+1;

    for (int j=ymin; j<ymax; j++) {
        for (int i=xmin; i<xmax; i++) {
            if (j >= y) {
                if (hasChunkAt(i, z)) {
                    int64_t end_chunk = toKey(i, z);
                    setBlockAt(i, j, z, EMPTY);
                    if (chunks.count(end_chunk) == 0) {
                        chunks.insert({end_chunk, 0});
                    }
                }
            } else {
                float f = roundedCone(vec3(i, j, z), c1, c2, r1-.1, r2-.1);
                if (f <= 0 && hasChunkAt(i, z)) {
                    int64_t end_chunk = toKey(i, z);
                    setBlockAt(i, j, z, WATER);
                    if (chunks.count(end_chunk) == 0) {
                        chunks.insert({end_chunk, 0});
                    }
                }
            }
        }
    }
//    for (int i=xmin; i<xmax; i++) {
//        if (hasChunkAt(i, z)) {
////            std::cout << "Filling : " << x << "," << i << std::endl;
//            int64_t end_chunk = toKey(i, z);
//            setBlockAt(i, 180, z, STONE);
//            if (chunks.count(end_chunk) == 0) {
//                chunks.insert({end_chunk, 0});
//            }
//        }
//    }
}

void Terrain::fillAboveRiver(int z, int x1, int x2, int y, int r1, int r2, vec3 c1, vec3 c2, std::map<int64_t, int>& chunks) {
    int xmin = std::min(x1,x2);
    int xmax = std::max(x1,x2);

    for (int i=xmin; i<xmax; i++) {
        //the smaler the f value, the lower the height
        int j = y;
        float f = roundedCone(vec3(i, j, z), c1, c2, r1-.1, r2-.1);
        while (1) {
            if (hasChunkAt(i,z)) {
                BlockType b = getBlockAt(i, j, z);
                if (b == EMPTY) {
                    break;
                }
                j++;
            } else {
                break;
            }
        }
        if (f > 0) {
            if (hasChunkAt(i, z)) {
                float scale = f / 5.f;
                float diff = j - y;
                diff *= scale;
    //            std::cout << "x,z " << x << "," << i << " F: " << f << " ymax: " << j << " New max: "
    //                      << y+std::floor(diff) << std::endl;
                std::pair<int, Biome> temp = HeightMap::getHeight(i, z);
                setBlockAt(i, y+std::floor(diff), z, BlockTypeWorker::m_topBlockMap[temp.second]);
                for (int k=y+std::floor(diff)+1; k<j; k++) {
                    if (hasChunkAt(i, z)) { //inside circle
                       int64_t end_chunk = toKey(i, z);
                       setBlockAt(i, k, z, EMPTY);
                       if (chunks.count(end_chunk) == 0) {
                           chunks.insert({end_chunk, 0});
                       }
                   }
                }
            }
        }
    }
}

void Terrain::carveSeg(ivec2 start, ivec2 end, int r1, int r2, int y1, int y2, std::map<int64_t, int>& chunks) {
    float bound = std::max(r1, r2);
    vec2 core = end-start;
    vec3 cross = normalize(glm::cross(vec3(core.x, core.y, 0.f), vec3(0.f, 0.f, 1.f)));
//    std::cout << "Cross: " << to_string(cross) << std::endl;
    vec2 perp = vec2(cross.x, cross.y);

    vec2 a = vec2(start) + perp * bound;
    vec2 d = vec2(start) - perp * bound;
    vec2 b = vec2(end) + perp * bound;
    vec2 c = vec2(end) - perp * bound;

//    std::cout << "Direction " << to_string(start) << ", " << to_string(end) << std::endl;
//    std::cout << "Corners " << to_string(a) << " " << to_string(b) << " " << to_string(c) << " " << to_string(d) << std::endl;

    vec2 bigA = vec2(start) + perp * (bound + 4);
    vec2 bigD = vec2(start) - perp * (bound + 4);
    vec2 bigB = vec2(end) + perp * (bound + 4);
    vec2 bigC = vec2(end) - perp * (bound + 4);

    vec3 c1 = vec3(start.x, y1-1, start.y);
    vec3 c2 = vec3(end.x, y2-1, end.y);

    LineSegment ab = LineSegment(a, b);
    LineSegment bc = LineSegment(b, c);
    LineSegment cd = LineSegment(c, d);
    LineSegment da = LineSegment(d, a);

    LineSegment bigAB = LineSegment(bigA, bigB);
    LineSegment bigBC = LineSegment(bigB, bigC);
    LineSegment bigCD = LineSegment(bigC, bigD);
    LineSegment bigDA = LineSegment(bigD, bigA);

    std::map<int, float> entry;
    std::map<int, float> exit;
    std::map<int, float> bigEntry;
    std::map<int, float> bigExit;

    rasterizeLine(ab, entry, exit);
    rasterizeLine(bc, entry, exit);
    rasterizeLine(cd, entry, exit);
    rasterizeLine(da, entry, exit);

    rasterizeLine(bigAB, bigEntry, bigExit);
    rasterizeLine(bigBC, bigEntry, bigExit);
    rasterizeLine(bigCD, bigEntry, bigExit);
    rasterizeLine(bigDA, bigEntry, bigExit);

    for (auto& a : entry) {
        float ex = exit[a.first];
        fillRiver(a.first, std::round(a.second), std::round(ex), (y1+y2)/2, r1, r2, c1, c2, chunks);
    }
    for (auto& a : bigEntry) {
        float ex = bigExit[a.first];
        fillAboveRiver(a.first, std::round(a.second), std::round(ex), (y1+y2)/2, r1, r2, c1, c2, chunks);
    }
}

void Terrain::CreateTestScene()
{
    // Create the Chunks that will
    // store the blocks for our
    // initial world space
    QSet<int64_t> initBorderingZones = zonesBorderingZone(ivec2(0,0), 2);
    for (auto id : initBorderingZones) {
        spawnBlockTypeWorker(id);
    }
    QThreadPool::globalInstance()->waitForDone();

    m_chunksWithBlockDataMux.lock();
    spawnVBOWorkers(m_chunksWithBlockData);
    m_chunksWithBlockData.clear();
    m_chunksWithBlockDataMux.unlock();
    QThreadPool::globalInstance()->waitForDone();

    m_chunksWithVBODataMux.lock();
    for (uPtr<ChunkVBOData>& c : m_chunksWithVBOData) {
        c->mp_chunk->createVBOdata(c->m_idxDataOpaque, c->m_vboDataOpaque);
        c->mp_chunk->createVBOdataTransparent(c->m_idxDataTransparent, c->m_vboDataTransparent);
    }
    m_chunksWithVBOData.clear();
    m_chunksWithVBODataMux.unlock();
    m_initialSceneLoaded = true;
}


