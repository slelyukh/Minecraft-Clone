#include "mygl.h"
#include <glm_includes.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>


MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_worldAxes(this),
      m_progLambert(this), m_progFlat(this), m_progInstanced(this), m_textureAtlas(this), m_nMap(this),
      m_progShadows(this), m_postProcess(this), frameBuff(this, width()*devicePixelRatio(), height()*devicePixelRatio(), devicePixelRatio()),
      shadowMap(this, 1024, 1024, 1),
      m_geomQuad(this),
      m_terrain(this), m_player(glm::vec3(48.f, 140.f, 48.f), m_terrain),

      currentT(QDateTime::currentMSecsSinceEpoch()), shaderT(0.f)
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
    // Tell the timer to redraw 60 times per second
    m_timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
//    setCursor(Qt::BlankCursor); // Make the cursor invisible
}

MyGL::~MyGL() {
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
}


void MyGL::moveMouseToCenter() {
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instance of the world axes
    m_worldAxes.createVBOdata();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    //m_progInstanced.create(":/glsl/instanced.vert.glsl", ":/glsl/lambert.frag.glsl");
    m_postProcess.create(":/glsl/passthrough.vert.glsl", ":/glsl/water.frag.glsl");
    m_progShadows.create(":/glsl/shadowMapping.vert.glsl", ":/glsl/shadowMapping.frag.glsl");

    // Set a color with which to draw geometry.
    // This will ultimately not be used when you change
    // your program to render Chunks with vertex colors
    // and UV coordinates
    m_progLambert.setGeometryColor(glm::vec4(0,1,0,1));

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);

    //frameBuff.create();
    //shadowMap.setFrameBuffer(m_progShadows.unifShadowMap);
    m_geomQuad.createVBOdata();

    m_textureAtlas.create(":/textures/minecraft_textures_all.png");
    m_textureAtlas.bind(0);
    m_nMap.create(":/textures/minecraft_normals_all.png");
    m_nMap.bind(1);

    shadowMap.create();
    shadowMap.bindFrameBuffer();
    shadowMap.bindToTextureSlot(2);

    m_progLambert.setSampler(0);
    m_progLambert.setNMap(1);
    m_progLambert.setSMap(2);

    m_progShadows.setSampler(0);
    m_progShadows.setNMap(1);
    m_progShadows.setSMap(2);
    m_postProcess.setSampler(0);

    m_terrain.CreateTestScene();
    //m_terrain.createRiver(ivec2(0,0), 45.f);
}

void MyGL::resizeGL(int w, int h) {
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_player.setCameraWidthHeight(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);
    m_postProcess.setViewProjMatrix(viewproj);

    frameBuff.resize(width()*devicePixelRatio(), height()*devicePixelRatio(), devicePixelRatio());

    printGLErrorLog();
}


// MyGL's constructor links tick() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to perform
// all per-frame actions here, such as performing physics updates on all
// entities in the scene.
void MyGL::tick() {
    float dt = (QDateTime::currentMSecsSinceEpoch() - currentT) / 1000.f;
    currentT = QDateTime::currentMSecsSinceEpoch();

    shaderT += dt;
    m_progLambert.setTime(shaderT);

    m_player.tick(dt, m_inputs);
    m_terrain.multiThreadedWork(m_player.mcr_position, m_player.mcr_posPrev, dt);
    //m_inputs = InputBundle(); //reset input bundle
    update(); // Calls paintGL() as part of a larger QOpenGLWidget pipeline
    sendPlayerDataToGUI(); // Updates the info in the secondary window displaying player data


    //reset mouse position to center
    //0 out input data structure to prevent infinite rotation
    m_inputs.mouseX = 0.f;
    m_inputs.mouseY = 0.f;
}

void MyGL::sendPlayerDataToGUI() const {
    emit sig_sendPlayerPos(m_player.posAsQString());
    emit sig_sendPlayerVel(m_player.velAsQString());
    emit sig_sendPlayerAcc(m_player.accAsQString());
    emit sig_sendPlayerLook(m_player.lookAsQString());
    glm::vec2 pPos(m_player.mcr_position.x, m_player.mcr_position.z);
    glm::ivec2 chunk(16 * glm::ivec2(glm::floor(pPos / 16.f)));
    glm::ivec2 zone(64 * glm::ivec2(glm::floor(pPos / 64.f)));
    emit sig_sendPlayerChunk(QString::fromStdString("( " + std::to_string(chunk.x) + ", " + std::to_string(chunk.y) + " )"));
    emit sig_sendPlayerTerrainZone(QString::fromStdString("( " + std::to_string(zone.x) + ", " + std::to_string(zone.y) + " )"));
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL() {
    //frameBuff.bindFrameBuffer();
    //glViewport(0,0,width()*devicePixelRatio(), height()*devicePixelRatio());
    // Clear the screen so that we only see newly drawn images
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progLambert.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progInstanced.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_postProcess.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progShadows.setViewProjMatrix(m_player.mcr_camera.getViewProj());

    m_progShadows.setDepth(m_player.mcr_lightSource.getViewProj());
    m_progLambert.setDepth(m_player.mcr_lightSource.getViewProj());



    renderTerrain();
    //post process step
    //reset frame buffer output
//    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
//    glViewport(0,0,width()*devicePixelRatio(), height()*devicePixelRatio());
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    frameBuff.bindToTextureSlot(1);

//    m_postProcess.drawInterleaved(m_geomQuad);

    glDisable(GL_DEPTH_TEST);
    m_progFlat.setModelMatrix(glm::mat4());
    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progFlat.draw(m_worldAxes);
    glEnable(GL_DEPTH_TEST);
}

// TODO: Change this so it renders the nine zones of generated
// terrain that surround the player (refer to Terrain::m_generatedTerrain
// for more info)
void MyGL::renderTerrain() {
    // find which terrain generation zone I am in
    glm::vec3 currentTGZ = m_player.mcr_position;
    currentTGZ.x = glm::floor(currentTGZ.x/64.f)*64.f;
    currentTGZ.z = glm::floor(currentTGZ.z/64.f)*64.f;

    shadowMap.bindFrameBuffer();
    glViewport(0,0, 1024, 1024);
    //glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
    //glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_terrain.drawOpaque(currentTGZ.x-128, currentTGZ.x+192, currentTGZ.z-128, currentTGZ.z+192, &m_progShadows);

    shadowMap.load(2);
    m_textureAtlas.load(0);
    m_nMap.load(1);
    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_terrain.draw(currentTGZ.x-128, currentTGZ.x+192, currentTGZ.z-128, currentTGZ.z+192, &m_progLambert);
}

void MyGL::keyPressEvent(QKeyEvent *e) {
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = true;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = true;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = true;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = true;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = true;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = true;
    } else if (e->key() == Qt::Key_F) {
        m_player.flight = !m_player.flight;
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = true;
    }
}

void MyGL::keyReleaseEvent(QKeyEvent *e) {
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead
    if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = false;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = false;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = false;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = false;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = false;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = false;
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = false;
    }
}

void MyGL::mouseMoveEvent(QMouseEvent *e) {
    // TODO
    QPoint p = e->pos();
    static const QPoint center = this->mapToGlobal(QPoint(width() / 2, height() / 2));
    m_inputs.mouseX = (p.x() - width()/2.f) / (float) width() * -90.f * .15;
    m_inputs.mouseY = (p.y() - height()/2.f) / (float) height() * -90.f * .15;
    moveMouseToCenter();
//    std::cout << m_inputs.mouseX << ", " << m_inputs.mouseY << std::endl;
//    std::cout << width() << ", " << height() << std::endl;
}

void MyGL::mousePressEvent(QMouseEvent *e) {
    // TODO
    if (e->button() == Qt::LeftButton) {
        m_inputs.leftClick = true;
    }
    if (e->button() == Qt::RightButton) {
        m_inputs.rightClick = true;
    }
}
