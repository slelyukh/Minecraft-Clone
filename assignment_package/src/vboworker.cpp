#include "vboworker.h"
using namespace std;

VBOWorker::VBOWorker(Chunk* c, std::vector<uPtr<ChunkVBOData>> &chunks,
                     QMutex* mv)
: m_chunk(c), m_chunks(chunks), vboMux(mv)
{}

void VBOWorker::run() {
    uPtr<ChunkVBOData> data = mkU<ChunkVBOData>(m_chunk);
    for(int i = 0; i < 16; ++i) {
        for(int j = 0; j < 256; ++j) {
            for(int k = 0; k < 16; ++k) {
            BlockType t = m_chunk->getBlockAt(i, j, k);
                if(t != EMPTY) {
                    BlockType ip;
                    BlockType in;
                    BlockType jp;
                    BlockType jn;
                    BlockType kp;
                    BlockType kn;
                    if (i == 15) {
                        if (m_chunk->mcr_neighbors.at(XPOS)) {
                            ip = m_chunk->mcr_neighbors.at(XPOS)->getBlockAt(0, j, k);
                        } else {
                            ip = EMPTY;
                        }
                    } else {
                        ip = m_chunk->getBlockAt(i+1, j, k);
                    }
                    if (i == 0) {
                        if (m_chunk->mcr_neighbors.at(XNEG)) {
                            in = m_chunk->mcr_neighbors.at(XNEG)->getBlockAt(15, j, k);
                        } else {
                            in = EMPTY;
                        }
                    } else {
                        in = m_chunk->getBlockAt(i-1, j, k);
                    }
                    if (j == 255) {
                        jp = EMPTY;
                    } else {
                        jp = m_chunk->getBlockAt(i, j+1, k);
                    }
                    if (j == 0) {
                        jn = EMPTY;
                    } else {
                        jn = m_chunk->getBlockAt(i, j-1, k);
                    }
                    if (k == 15) {
                        if (m_chunk->mcr_neighbors.at(ZPOS)) {
                            kp = m_chunk->mcr_neighbors.at(ZPOS)->getBlockAt(i, j, 0);
                        } else {
                            kp = EMPTY;
                        }
                    } else {
                        kp = m_chunk->getBlockAt(i, j, k+1);
                    }
                    if (k == 0) {
                        if (m_chunk->mcr_neighbors.at(ZNEG)) {
                            kn = m_chunk->mcr_neighbors.at(ZNEG)->getBlockAt(i, j, 15);
                        } else {
                            kn = EMPTY;
                        }
                    } else {
                        kn = m_chunk->getBlockAt(i, j, k-1);
                    }
                    if (t == WATER || t == ICE || t == CACTUS) {
                        m_chunk->createVBOdataCube(t, ip, in, jp, jn, kp, kn, glm::vec4(i, j, k, 0.f), data->m_idxDataTransparent, data->m_vboDataTransparent);
                    } else {
                        m_chunk->createVBOdataCube(t, ip, in, jp, jn, kp, kn, glm::vec4(i, j, k, 0.f), data->m_idxDataOpaque, data->m_vboDataOpaque);
                    }
                }
            }
        }
    }
    vboMux->lock();
    m_chunks.push_back(move(data));
    vboMux->unlock();
}
