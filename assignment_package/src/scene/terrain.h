#pragma once
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include "chunk.h"
#include <array>
#include <unordered_map>
#include <QThreadPool>
#include <unordered_set>
#include "shaderprogram.h"
#include "heightmap.h"
#include "vboworker.h"
#include "blocktypeworker.h"
#include "lsystem.h"
#include <stack>

using namespace glm;


//using namespace std;

// Helper functions to convert (x, z) to and from hash map key
int64_t toKey(int x, int z);
glm::ivec2 toCoords(int64_t k);

struct ChunkVBOData;
enum BlockType: unsigned char;

// The container class for all of the Chunks in the game.
// Ultimately, while Terrain will always store all Chunks,
// not all Chunks will be drawn at any given time as the world
// expands.
class Terrain {
private:
    // Stores every Chunk according to the location of its lower-left corner
    // in world space.
    // We combine the X and Z coordinates of the Chunk's corner into one 64-bit int
    // so that we can use them as a key for the map, as objects like std::pairs or
    // glm::ivec2s are not hashable by default, so they cannot be used as keys.
    std::unordered_map<int64_t, uPtr<Chunk>> m_chunks;

    // We will designate every 64 x 64 area of the world's x-z plane
    // as one "terrain generation zone". Every time the player moves
    // near a portion of the world that has not yet been generated
    // (i.e. its lower-left coordinates are not in this set), a new
    // 4 x 4 collection of Chunks is created to represent that area
    // of the world.
    // The world that exists when the base code is run consists of exactly
    // one 64 x 64 area with its lower-left corner at (0, 0).
    // When milestone 1 has been implemented, the Player can move around the
    // world to add more "terrain generation zone" IDs to this set.
    // While only the 3 x 3 collection of terrain generation zones
    // surrounding the Player should be rendered, the Chunks
    // in the Terrain will never be deleted until the program is terminated.
    std::unordered_set<int64_t> m_generatedTerrain;
    QMutex m_genTerrainMux;
    std::vector<uPtr<ChunkVBOData>> m_chunksWithVBOData;
    QMutex m_chunksWithVBODataMux;
    std::unordered_set<Chunk*> m_chunksWithBlockData;
    QMutex m_chunksWithBlockDataMux;

    float m_tryExpansionTimer;
    boolean m_initialSceneLoaded;
    OpenGLContext* mp_context;

public:
    Terrain(OpenGLContext *context);
    static int64_t toKey(int x, int z);
    ~Terrain();

    // Instantiates a new Chunk and stores it in
    // our chunk map at the given coordinates.
    // Returns a pointer to the created Chunk.
    Chunk* instantiateChunkAt(int x, int z);
    // Do these world-space coordinates lie within
    // a Chunk that exists?
    bool hasChunkAt(int x, int z) const;
    // Assuming a Chunk exists at these coords,
    // return a mutable reference to it
    uPtr<Chunk>& getChunkAt(int x, int z);
    // Assuming a Chunk exists at these coords,
    // return a const reference to it
    const uPtr<Chunk>& getChunkAt(int x, int z) const;
    // Given a world-space coordinate (which may have negative
    // values) return the block stored at that point in space.
    BlockType getBlockAt(int x, int y, int z) const;
    BlockType getBlockAt(glm::vec3 p) const;
    // Given a world-space coordinate (which may have negative
    // values) set the block at that point in space to the
    // given type.
    void setBlockAt(int x, int y, int z, BlockType t);

    void spawnVBOWorkers(const std::unordered_set<Chunk*> &);
    void spawnBTWorkers(const std::unordered_set<int64_t> &);

    void checkThreadResults();
    QSet<int64_t> zonesBorderingZone(ivec2, int );
    boolean terrainZoneExists(int64_t);
    void tryExpansion(glm::vec3, glm::vec3);
    void multiThreadedWork(glm::vec3, glm::vec3, float);

    // Draws every Chunk that falls within the bounding box
    // described by the min and max coords, using the provided
    // ShaderProgram
    void draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram);
    void drawOpaque(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram);

    void spawnVBOWorker(Chunk*);
    void spawnBlockTypeWorker(int64_t);

    void createVBOData(Chunk*);

    void createRiver(ivec2, float);
    void carveSeg(ivec2, ivec2, int r1, int r2, int y1, int y2, std::map<int64_t, int>& chunks);
    void fillRiver(int, int, int, int, int, int, vec3, vec3, std::map<int64_t, int>& chunks);
    void fillAboveRiver(int, int, int, int, int, int, vec3, vec3, std::map<int64_t, int>& chunks);

    // Initializes the Chunks that store the 64 x 256 x 64 block scene you
    // see when the base code is run.
    void CreateTestScene();
};
