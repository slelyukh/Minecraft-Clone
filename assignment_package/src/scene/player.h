#pragma once
#include "entity.h"
#include "camera.h"
#include "terrain.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <math.h>


class Player : public Entity {
private:
    glm::vec3 m_velocity, m_acceleration;
    Camera m_camera;
    Camera m_lightSource;
    Terrain &mcr_terrain;

    void processInputs(InputBundle &inputs);
    void computePhysics(float dT, const Terrain &terrain, bool jumped);
    void checkSubmerge(const Terrain &terrain);
public:
    // Readonly public reference to our camera
    // for easy access from MyGL
    const Camera& mcr_camera;
    const Camera& mcr_lightSource;

    //Flight mode
    bool flight;
    //Vertical rotation
    float theta;
    BlockType submerge;

    Player(glm::vec3 pos, Terrain &terrain);
    virtual ~Player() override;

    void setCameraWidthHeight(unsigned int w, unsigned int h);

    void tick(float dT, InputBundle &input) override;

    // Player overrides all of Entity's movement
    // functions so that it transforms its camera
    // by the same amount as it transforms itself.
    void moveAlongVector(glm::vec3 dir) override;
    void moveForwardLocal(float amount) override;
    void moveRightLocal(float amount) override;
    void moveUpLocal(float amount) override;
    void moveForwardGlobal(float amount) override;
    void moveRightGlobal(float amount) override;
    void moveUpGlobal(float amount) override;
    void rotateOnForwardLocal(float degrees) override;
    void rotateOnRightLocal(float degrees) override;
    void rotateOnUpLocal(float degrees) override;
    void rotateOnForwardGlobal(float degrees) override;
    void rotateOnRightGlobal(float degrees) override;
    void rotateOnUpGlobal(float degrees) override;

    // For sending the Player's data to the GUI
    // for display
    QString posAsQString() const;
    QString velAsQString() const;
    QString accAsQString() const;
    QString lookAsQString() const;
};
