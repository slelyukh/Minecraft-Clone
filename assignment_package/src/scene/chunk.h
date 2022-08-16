#pragma once
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include "drawable.h"
#include "terrain.h"
#include <array>
#include <unordered_map>
#include <cstddef>

struct ChunkVBOData;

//using namespace std;

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY, GRASS, SNOWGRASS, DIRT, STONE, WATER, SNOW, LAVA, BEDROCK, ICE,
    SAND, SANDSTONE, OBSIDIAN, CACTUS, COBBLE, WOOD, LEAVES, SHIT
};

// The six cardinal directions in 3D space
enum Direction : unsigned char
{
    XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG
};

// Lets us use any enum class as the key of a
// std::unordered_map
struct EnumHash {
    template <typename T>
    size_t operator()(T t) const {
        return static_cast<size_t>(t);
    }
};

// One Chunk is a 16 x 256 x 16 section of the world,
// containing all the Minecraft blocks in that area.
// We divide the world into Chunks in order to make
// recomputing its VBO data faster by not having to
// render all the world at once, while also not having
// to render the world block by block.

// TODO have Chunk inherit from Drawable
class Chunk : public Drawable {
private:
    // All of the blocks contained within this Chunk
    std::unordered_map<Direction, Chunk*, EnumHash> m_neighbors;
    std::array<BlockType, 65536> m_blocks;
    // This Chunk's four neighbors to the north, south, east, and west
    // The third input to this map just lets us use a Direction as
    // a key for this map.
    // These allow us to properly determine
    OpenGLContext *mp_context;
    boolean m_VBOcreated; //true when both opaque and transparent
    boolean m_VBOcreatedO; //opaque
    boolean m_VBOcreatedT; //transparent

public:
    Chunk(OpenGLContext *context);
    virtual ~Chunk() {};
    // A readonly reference to VBO status
    const boolean& mcr_VBOcreated;
    boolean m_blocksFilled;
    const std::unordered_map<Direction, Chunk*, EnumHash>& mcr_neighbors;
    void createVBOdata() override {};
    void createVBOdata(std::vector<GLuint> &, std::vector<glm::vec4> &);
    void createVBOdataTransparent(std::vector<GLuint> &, std::vector<glm::vec4> &);
    void createVBOdataCube(BlockType, BlockType, BlockType, BlockType, BlockType, BlockType, BlockType,
                           glm::vec4, std::vector<GLuint> &, std::vector<glm::vec4> &);
    void createVBOdataFace(glm::vec4 blockPos,  glm::vec4 v1, glm::vec4 v2, bool negNor, glm::vec4 color, glm::vec4 uv, std::vector<GLuint> &ch_idx, std::vector<glm::vec4> &ch_vert_data, float size = 1, float uvSize = 0.0625f);
    void destroy();
    BlockType getBlockAt(unsigned int x, unsigned int y, unsigned int z) const;
    BlockType getBlockAt(int x, int y, int z) const;
    void setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void linkNeighbor(uPtr<Chunk>& neighbor, Direction dir);
};
