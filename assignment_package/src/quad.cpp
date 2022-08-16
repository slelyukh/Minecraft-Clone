#include "quad.h"

Quad::Quad(OpenGLContext *context) : Drawable(context)
{}

void Quad::createVBOdata()
{
    GLuint idx[6]{0, 1, 2, 0, 2, 3};
    glm::vec4 vert_data[16] {glm::vec4(-1.f, -1.f, -0.99f, 1.f),
                           glm::vec4(0, 0, 1, 1),
                            glm::vec4(0, 0, 0, 0),
                            glm::vec4(0, 1, 0, 0),

                           glm::vec4(1.f, -1.f, -0.99f, 1.f),
                           glm::vec4(0, 0, 1, 1),
                            glm::vec4(1, 0, 0, 0),
                            glm::vec4(0, 1, 0, 0),

                           glm::vec4(1.f, 1.f, -0.99f, 1.f),
                           glm::vec4(0, 0, 1, 1),
                            glm::vec4(1, 1, 0, 0),
                            glm::vec4(0, 1, 0, 0),

                           glm::vec4(-1.f, 1.f, -0.99f, 1.f),
                           glm::vec4(0, 0, 1, 1),
                            glm::vec4(0, 1, 0, 0),
                            glm::vec4(0, 1, 0, 0)};

    m_count = 6;

    // Create a VBO on our GPU and store its handle in bufIdx
    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), idx, GL_STATIC_DRAW);

    generateInter();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufInter);
    mp_context->glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(glm::vec4), vert_data, GL_STATIC_DRAW);
}
