#pragma once
#include <QRunnable>
#include <QMutex>
#include <unordered_set>
#include "scene/chunk.h"
#include "scene/terrain.h"
using namespace std;
class Chunk;

struct ChunkVBOData {
    Chunk* mp_chunk;
    std::vector<glm::vec4> m_vboDataOpaque;
    std::vector<GLuint> m_idxDataOpaque;
    std::vector<glm::vec4> m_vboDataTransparent;
    std::vector<GLuint> m_idxDataTransparent;

    ChunkVBOData(Chunk* c)
     : mp_chunk(c), m_vboDataOpaque(), m_idxDataOpaque(),
       m_vboDataTransparent(), m_idxDataTransparent()
    {}
};

class VBOWorker : public QRunnable {
private:
    Chunk* m_chunk;
    std::vector<uPtr<ChunkVBOData>>& m_chunks;
    QMutex *vboMux;

public:
    VBOWorker(Chunk*, std::vector<uPtr<ChunkVBOData>> &,
              QMutex*);
    void run() override;
};
