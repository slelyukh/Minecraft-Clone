#include "chunk.h"
#include <iostream>

Chunk::Chunk(OpenGLContext *context) : Drawable(context),
  m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}},
  m_blocks(),
  mp_context(context), m_VBOcreated(false), m_VBOcreatedO(false), m_VBOcreatedT(false),
  mcr_VBOcreated(m_VBOcreated), m_blocksFilled(false), mcr_neighbors(m_neighbors)
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

void Chunk::destroy() {
    this->destroyVBOdata();
    m_VBOcreated = false;
}

// Does bounds checking with at()
BlockType Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z) const {
    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getBlockAt(int x, int y, int z) const {

    return getBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));

}

// Does bounds checking with at()
void Chunk::setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
}


const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection {
    {XPOS, XNEG},
    {XNEG, XPOS},
    {YPOS, YNEG},
    {YNEG, YPOS},
    {ZPOS, ZNEG},
    {ZNEG, ZPOS}
};

void Chunk::linkNeighbor(uPtr<Chunk> &neighbor, Direction dir) {
    if(neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
    }
}

void Chunk::createVBOdataFace(glm::vec4 blockPos,  glm::vec4 v1, glm::vec4 v2, bool norNeg, glm::vec4 color, glm::vec4 uv, std::vector<GLuint> &ch_idx, std::vector<glm::vec4> &ch_vert_data, float size, float uvSize) {
        blockPos.w = 1.f;
        GLuint i = ch_vert_data.size()/4;
        v1.w = 0;
        v2.w = 0;
        v1 = glm::normalize(v1);
        v2 = glm::normalize(v2);
        glm::vec4 nor = glm::vec4(glm::cross(glm::vec3(v1.x, v1.y, v1.z), glm::vec3(v2.x, v2.y, v2.z)), 1);
        if (norNeg) {
            nor = -nor;
        }

        ch_vert_data.push_back(blockPos); //pos
        ch_vert_data.push_back(nor); //nor
        ch_vert_data.push_back(uv);
        ch_vert_data.push_back(color);

        ch_vert_data.push_back(blockPos + size*v1); //pos
        ch_vert_data.push_back(nor); //nor
        ch_vert_data.push_back(uv + glm::vec4(0, uvSize, 0, 0));
        ch_vert_data.push_back(color);

        ch_vert_data.push_back(blockPos + size*(v1+v2)); //pos
        ch_vert_data.push_back(nor); //nor
        ch_vert_data.push_back(uv + glm::vec4(uvSize, uvSize, 0, 0));
        ch_vert_data.push_back(color);

        ch_vert_data.push_back(blockPos + size*v2); //pos
        ch_vert_data.push_back(nor); //nor
        ch_vert_data.push_back(uv + glm::vec4(uvSize, 0, 0, 0));
        ch_vert_data.push_back(color);

        ch_idx.push_back(i);
        ch_idx.push_back(i+1);
        ch_idx.push_back(i+3);
        ch_idx.push_back(i+2);
        ch_idx.push_back(i+3);
        ch_idx.push_back(i+1);
}

void Chunk::createVBOdataCube(BlockType t, BlockType ip, BlockType in, BlockType jp, BlockType jn, BlockType kp, BlockType kn,
                       glm::vec4 blockPos, std::vector<GLuint> &ch_idx, std::vector<glm::vec4> &ch_vert_data) {
                    glm::vec4 uvTop = glm::vec4(0, 0, 0, 0); //z = 1 if animated w = 1 if transparent
                    glm::vec4 uvBot = glm::vec4(0, 0, 0, 0);
                    glm::vec4 uv = glm::vec4(0, 0, 0, 0);
                    glm::vec4 color = glm::vec4(0, 0, 0, 0);
                    float idx = 0.0625f;
                    switch(t) {
                        case GRASS:
                            uvTop = glm::vec4(0.5, 0.8125, 0, 0);
                            uvBot = glm::vec4(0.125, 0.9375, 0, 0);
                            uv = glm::vec4(0.1825, 0.9375, 0, 0);
                            break;
                        case DIRT:
                            uvTop = glm::vec4(0.125, 0.9375, 0, 0);
                            uvBot = glm::vec4(0.125, 0.9375, 0, 0);
                            uv = glm::vec4(0.125, 0.9375, 0, 0);
                            break;
                        case STONE:
                            uvTop = glm::vec4(0.0625, 0.9375, 0, 0);
                            uvBot = glm::vec4(0.0625, 0.9375, 0, 0);
                            uv = glm::vec4(0.0625, 0.9375, 0, 0);
                            break;
                        case WATER:
                            uvTop = glm::vec4(0.8125, 0.1875, 1, 1);
                            uvBot = glm::vec4(0.8125, 0.1875, 1, 1);
                            uv = glm::vec4(0.8125, 0.1875, 1, 1);
                            break;
                        case LAVA:
                            uvTop = glm::vec4(0.8125, 0.0625, 1, 0);
                            uvBot = glm::vec4(0.8125, 0.0625, 1, 0);
                            uv = glm::vec4(0.8125, 0.0625, 1, 0);
                            break;
                        case SNOW:
                            uvTop = glm::vec4(0.125, 0.6875, 0, 0);
                            uvBot = glm::vec4(0.125, 0.6875, 0, 0);
                            uv = glm::vec4(0.125, 0.6875, 0, 0);
                            break;
                        case ICE:
                            uv = glm::vec4(3*idx, 11*idx, 0, 1);
                            uvTop = uv;
                            uvBot = uv;
                            break;
                        case COBBLE:
                            uv = glm::vec4(0, 14*idx, 0, 0);
                            uvTop = uv;
                            uvBot = uv;
                            break;
                        case SANDSTONE:
                            uv = glm::vec4(0, 3*idx, 0, 0);
                            uvTop = glm::vec4(0, 4*idx, 0, 0);
                            uvBot = glm::vec4(0, 2*idx, 0, 0);
                            break;
                        case SAND:
                            uv = glm::vec4(2*idx, 14*idx, 0, 0);
                            uvTop = uv;
                            uvBot = uv;
                            break;
                        case OBSIDIAN:
                            uv = glm::vec4(7*idx, 4*idx, 0, 0);
                            uvTop = uv;
                            uvBot = uv;
                            break;
                        case BEDROCK:
                            uv = glm::vec4(1*idx, 14*idx, 0, 0);
                            uvTop = uv;
                            uvBot = uv;
                            break;
                        case CACTUS:
                            uv = glm::vec4(6*idx, 11*idx, 0, 1);
                            uvTop = glm::vec4(5*idx, 11*idx, 0, 1);
                            uvBot = uvTop;
                            break;
                        case SNOWGRASS:
                            uv = glm::vec4(4*idx, 11*idx, 0, 0);
                            uvTop = glm::vec4(2*idx, 11*idx, 0, 0);
                            uvBot = glm::vec4(2*idx, 15*idx, 0, 0);
                            break;
                        case WOOD:
                            uv = glm::vec4(4*idx, 13*idx, 0, 0);
                            uvTop = glm::vec4(5*idx, 13*idx, 0, 0);
                            uvBot = glm::vec4(5*idx, 13*idx, 0, 0);
                            break;
                        case LEAVES:
                            uv = glm::vec4(5*idx, 12*idx, 0, 0);
                            uvTop = uv;
                            uvBot = uv;
                            break;
                        case SHIT:
                            uv = glm::vec4(13*idx, 11*idx, 0, 0);
                            uvTop = glm::vec4(14*idx, 11*idx, 0, 0);
                            uvBot = glm::vec4(2*idx, 15*idx, 0, 0);
                            break;
                        case EMPTY:
                            break;
                        default:
                             // Other block types are not yet handled, so we default to debug purple
                             break;
                    }
                    float eOff = 0;
                    if (t == CACTUS) {
                        eOff = 1/16.f;
                    }
                    if (ip == EMPTY || ((ip == WATER || ip == ICE) && t != WATER && t != ICE)) { //right face
                        createVBOdataFace(blockPos + glm::vec4(1-eOff, 0, 0, 0), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 0.f, 1.f, 0.f), false, color, uv, ch_idx, ch_vert_data);
                    }
                    if (in == EMPTY || ((in == WATER || in == ICE) && t != WATER && t != ICE)) { //left face
                        createVBOdataFace(blockPos + glm::vec4(eOff, 0, 0, 0), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(0.f, 0.f, 1.f, 0.f), true, color, uv, ch_idx, ch_vert_data);
                    }
                    if (jp == EMPTY || jp == CACTUS || ((jp == WATER|| jp == ICE) && t != WATER && t != ICE)) { //top face
                        createVBOdataFace(blockPos + glm::vec4(0, 1, 0, 0), glm::vec4(0.f, 0.f, 1.f, 0.f), glm::vec4(1.f, 0.f, 0.f, 0.f), false, color, uvTop, ch_idx, ch_vert_data);

                    }
                    if (jn == EMPTY || ((jn == WATER || jn == ICE) && (t != WATER && t != ICE))) { //bottom face
                        createVBOdataFace(blockPos, glm::vec4(0.f, 0.f, 1.f, 0.f), glm::vec4(1.f, 0.f, 0.f, 0.f), true, color, uvBot, ch_idx, ch_vert_data);
                    }
                    if (kp == EMPTY || ((kp == WATER || kp == ICE)  && t != WATER && t != ICE)) { //front face
                        createVBOdataFace(blockPos + glm::vec4(0, 0, 1 - eOff, 0), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(1.f, 0.f, 0.f, 0.f), true, color, uv, ch_idx, ch_vert_data);
                    }
                    if (kn == EMPTY || ((kn == WATER || kn == ICE) && t != WATER && t != ICE)) { //back face
                        createVBOdataFace(blockPos + glm::vec4(0, 0, eOff, 0), glm::vec4(0.f, 1.f, 0.f, 0.f), glm::vec4(1.f, 0.f, 0.f, 0.f), false, color, uv, ch_idx, ch_vert_data);
                    }
}

void Chunk::createVBOdata(std::vector<GLuint> &opaqueIdx, std::vector<glm::vec4> &opaqueData) {
    m_count = opaqueIdx.size();

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count * sizeof(GLuint),
                             opaqueIdx.data(), GL_STATIC_DRAW);

    generateInter();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufInter);
    mp_context->glBufferData(GL_ARRAY_BUFFER, opaqueData.size() * sizeof(glm::vec4),
                             opaqueData.data(), GL_STATIC_DRAW);

    m_VBOcreatedO = true;
    if (m_VBOcreatedT) {
        m_VBOcreated = true;
    }
//    m_VBOcreated = true;
}

void Chunk::createVBOdataTransparent(std::vector<GLuint> &transparentIdx, std::vector<glm::vec4> &transparentData) {

    m_countT = transparentIdx.size();

    generateIdxT();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxT);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_countT * sizeof(GLuint),
                             transparentIdx.data(), GL_STATIC_DRAW);

    generateInterT();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufInterT);
    mp_context->glBufferData(GL_ARRAY_BUFFER, transparentData.size() * sizeof(glm::vec4),
                             transparentData.data(), GL_STATIC_DRAW);

    m_VBOcreatedT = true;
    if (m_VBOcreatedT) {
        m_VBOcreated = true;
    }
}
