#include "player.h"
#include <QString>

#define WALK_SPEED 200.f
#define FLY_SPEED 300.f
#define FALL_SPEED 600.f

Player::Player(glm::vec3 pos, const Terrain &terrain)
    : Entity(pos), m_velocity(0,0,0), m_acceleration(0,0,0),
      m_camera(pos + glm::vec3(0, 1.5f, 0)), mcr_terrain(terrain),
      mcr_camera(m_camera), flight(true), theta(0.f)
{}

Player::~Player()
{}

void Player::tick(float dT, InputBundle &input) {
    processInputs(input);
    computePhysics(dT, mcr_terrain);
}

bool gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit) {
    float maxLen = glm::length(rayDirection); // Farthest we search
    glm::ivec3 currCell = glm::ivec3(glm::floor(rayOrigin));
    rayDirection = glm::normalize(rayDirection); // Now all t values represent world dist.

    float curr_t = 0.f;
    while(curr_t < maxLen) {
        float min_t = glm::sqrt(3.f);
        float interfaceAxis = -1; // Track axis for which t is smallest
        for(int i = 0; i < 3; ++i) { // Iterate over the three axes
            if(rayDirection[i] != 0) { // Is ray parallel to axis i?
                float offset = glm::max(0.f, glm::sign(rayDirection[i])); // See slide 5
                // If the player is *exactly* on an interface then
                // they'll never move if they're looking in a negative direction
                if(currCell[i] == rayOrigin[i] && offset == 0.f) {
                    offset = -1.f;
                }
                int nextIntercept = currCell[i] + offset;
                float axis_t = (nextIntercept - rayOrigin[i]) / rayDirection[i];
                axis_t = glm::min(axis_t, maxLen); // Clamp to max len to avoid super out of bounds errors
                if(axis_t < min_t) {
                    min_t = axis_t;
                    interfaceAxis = i;
                }
            }
        }
        if(interfaceAxis == -1) {
            throw std::out_of_range("interfaceAxis was -1 after the for loop in gridMarch!");
        }
        curr_t += min_t; // min_t is declared in slide 7 algorithm
        rayOrigin += rayDirection * min_t;
        glm::ivec3 offset = glm::ivec3(0,0,0);
        // Sets it to 0 if sign is +, -1 if sign is -
        offset[interfaceAxis] = glm::min(0.f, glm::sign(rayDirection[interfaceAxis]));
        currCell = glm::ivec3(glm::floor(rayOrigin)) + offset;
        // If currCell contains something other than EMPTY, return
        // curr_t
        BlockType cellType = terrain.getBlockAt(currCell.x, currCell.y, currCell.z);
        if(cellType != EMPTY) {
            *out_blockHit = currCell;
            *out_dist = glm::min(maxLen, curr_t);
            return true;
        }
    }
    *out_dist = glm::min(maxLen, curr_t);
    return false;
}



void Player::processInputs(InputBundle &inputs) {
    // TODO: Update the Player's velocity and acceleration based on the
    // state of the inputs.

    //Handle mouse movement
    float dtheta = inputs.mouseY;
    theta += dtheta;
    if (theta > 90.f) {
        float diff = 90.f - theta;
        dtheta += diff;
        theta = 90.f;
    }
    if (theta < -90.f) {
        float diff = -90.f - theta;
        dtheta += diff;
        theta = -90.f;
    }
    this->rotateOnUpGlobal(inputs.mouseX);
    this->rotateOnRightLocal(dtheta);

    //Handle clicks
    if (inputs.leftClick) {
        float dist;
        glm::ivec3 block;
        bool b = gridMarch(m_camera.mcr_position, glm::normalize(m_forward) * 3.f, mcr_terrain, &dist, &block);
        std::cout << "HERE" << std::endl;
        if (b) {
            std::cout << "BLOCK REMOVED" << std::endl;
//            mcr_terrain.setBlockAt(block.x,block.y,block.z,EMPTY);
            const uPtr<Chunk>& c = mcr_terrain.getChunkAt(block.x, block.z);
            c->setBlockAt(block.x,block.y,block.z,EMPTY);
        }
        inputs.leftClick = false;
    }
    if (inputs.rightClick) {
        float dist;
        glm::ivec3 block;
        bool b = gridMarch(m_camera.mcr_position, glm::normalize(m_forward) * 3.f, mcr_terrain, &dist, &block);
        if (b) {
            glm::vec3 p = m_camera.mcr_position + glm::normalize(m_forward) * dist;
        }
        inputs.rightClick = false;
    }

    //Handle keyboard inputs
    m_acceleration = glm::vec3(0.f);
    if (inputs.wPressed) {
        m_acceleration += m_forward;
    }
    if (inputs.aPressed) {
        m_acceleration -= m_right;
    }
    if (inputs.sPressed) {
        m_acceleration -= m_forward;
    }
    if (inputs.dPressed) {
        m_acceleration += m_right;
    }
    if (flight) {
        if (inputs.qPressed) {
            m_acceleration += m_up;
        }
        if (inputs.ePressed) {
            m_acceleration -= m_up;
        }
    } else if (inputs.spacePressed){

    }
}

void Player::computePhysics(float dT, const Terrain &terrain) {
    // TODO: Update the Player's position based on its acceleration
    // and velocity, and also perform collision detection.

    //normalize for multi inputs
    if (m_acceleration != glm::vec3(0.f)) {
        if (!flight) {
            m_acceleration = glm::normalize(glm::vec3(m_acceleration.x, 0.f, m_acceleration.z));
        } else {
            m_acceleration = glm::normalize(m_acceleration);
        }
    }

    std::vector<glm::vec3> corners;
    for (int i = 0; i < 3; i++) { //add each corner starting from bottom and increasing y by 1 each time
        glm::vec3 topLeft = m_position + glm::vec3(-.5, i, .5);
        glm::vec3 topRight = m_position + glm::vec3(.5, i, .5);
        glm::vec3 botLeft = m_position + glm::vec3(-.5, i, -.5);
        glm::vec3 botRight = m_position + glm::vec3(.5, i, -.5);
        corners.push_back(topLeft);
        corners.push_back(topRight);
        corners.push_back(botLeft);
        corners.push_back(botRight);
    }
    int m_isGrounded = 0; //0=empty, 1=water, 2=solid block
    // Iterate over the bottom four corners and check just below them
    for(int i = 0; i < 4; ++i) {
        glm::vec3 belowCorner = corners[i] - glm::vec3(0, 0.01f, 0);
        BlockType belowMe = terrain.getBlockAt(belowCorner);

        if (belowMe == WATER || belowMe == LAVA) {
            m_isGrounded = 1;
            break;
        }
        if (belowMe != EMPTY) {
            m_isGrounded = 2;
            m_velocity.y = 0.f;
            break;
        }
    }
    if (!flight) {
        if (m_isGrounded == 0) {
            m_acceleration.y = 0.f;
        } else {
            m_acceleration.y = -1.f;
        }
    }

    if (flight) {
        m_acceleration *= FLY_SPEED * dT;
    } else {
//        std::cout << "Accel " << glm::to_string(m_acceleration) << std::endl;
        m_acceleration.x *= WALK_SPEED * dT;
        m_acceleration.z *= WALK_SPEED * dT;
        m_acceleration.y *= FALL_SPEED * dT;
        if (m_isGrounded == 1) {
            m_acceleration *= 2.f/3.f;
        }
    }

    m_velocity += m_acceleration * dT;
    m_velocity *= .95;

    //Collision checking
    glm::vec3 direction = m_velocity * dT;
    glm::vec3 final_dir = glm::vec3(direction);
    glm::vec3 minDist = glm::abs(direction);
    for (auto& c : corners) {
        float x,y,z;
        glm::ivec3 block;
        gridMarch(c, glm::vec3(direction.x, 0.f, 0.f), terrain, &x, &block);
        gridMarch(c, glm::vec3(0.f, direction.y, 0.f), terrain, &y, &block);
        gridMarch(c, glm::vec3(0.f, 0.f, direction.z), terrain, &z, &block);
        if (x < minDist.x) {
//            std::cout << "X block" << std::endl;
            minDist.x = x;
            m_velocity.x = 0.f;
        }
        if (y < minDist.y) {
//            std::cout << "Y block" << std::endl;
            minDist.y = y;
            m_velocity.y = 0.f;
        }
        if (z < minDist.z) {
//            std::cout << "Z block" << std::endl;
            minDist.z = z;
            m_velocity.z = 0.f;
        }
    }
    if (minDist.x > .01) {
        final_dir.x = glm::sign(final_dir.x) * minDist.x - (0.01 * glm::sign(final_dir.x));
    } else {
        final_dir.x = 0.f;
    }
    if (minDist.y > .01) {
        final_dir.y = glm::sign(final_dir.y) * minDist.y - (0.01 * glm::sign(final_dir.y));
    } else {
        final_dir.y = 0.f;
    }
    if (minDist.z > .01) {
        final_dir.z = glm::sign(final_dir.z) * minDist.z - (0.01 * glm::sign(final_dir.z));
    } else {
        final_dir.z = 0.f;
    }
//    std::cout << glm::to_string(direction) << " " << glm::to_string(final_dir) << std::endl;
    this->moveAlongVector(final_dir);
}

void Player::setCameraWidthHeight(unsigned int w, unsigned int h) {
    m_camera.setWidthHeight(w, h);
}

void Player::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);
    m_camera.moveAlongVector(dir);
}
void Player::moveForwardLocal(float amount) {
    Entity::moveForwardLocal(amount);
    m_camera.moveForwardLocal(amount);
}
void Player::moveRightLocal(float amount) {
    Entity::moveRightLocal(amount);
    m_camera.moveRightLocal(amount);
}
void Player::moveUpLocal(float amount) {
    Entity::moveUpLocal(amount);
    m_camera.moveUpLocal(amount);
}
void Player::moveForwardGlobal(float amount) {
    Entity::moveForwardGlobal(amount);
    m_camera.moveForwardGlobal(amount);
}
void Player::moveRightGlobal(float amount) {
    Entity::moveRightGlobal(amount);
    m_camera.moveRightGlobal(amount);
}
void Player::moveUpGlobal(float amount) {
    Entity::moveUpGlobal(amount);
    m_camera.moveUpGlobal(amount);
}
void Player::rotateOnForwardLocal(float degrees) {
    Entity::rotateOnForwardLocal(degrees);
    m_camera.rotateOnForwardLocal(degrees);
}
void Player::rotateOnRightLocal(float degrees) {
    Entity::rotateOnRightLocal(degrees);
    m_camera.rotateOnRightLocal(degrees);
}
void Player::rotateOnUpLocal(float degrees) {
    Entity::rotateOnUpLocal(degrees);
    m_camera.rotateOnUpLocal(degrees);
}
void Player::rotateOnForwardGlobal(float degrees) {
    Entity::rotateOnForwardGlobal(degrees);
    m_camera.rotateOnForwardGlobal(degrees);
}
void Player::rotateOnRightGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);
    m_camera.rotateOnRightGlobal(degrees);
}
void Player::rotateOnUpGlobal(float degrees) {
    Entity::rotateOnUpGlobal(degrees);
    m_camera.rotateOnUpGlobal(degrees);
}

QString Player::posAsQString() const {
    std::string str("( " + std::to_string(m_position.x) + ", " + std::to_string(m_position.y) + ", " + std::to_string(m_position.z) + ")");
    return QString::fromStdString(str);
}
QString Player::velAsQString() const {
    std::string str("( " + std::to_string(m_velocity.x) + ", " + std::to_string(m_velocity.y) + ", " + std::to_string(m_velocity.z) + ")");
    return QString::fromStdString(str);
}
QString Player::accAsQString() const {
    std::string str("( " + std::to_string(m_acceleration.x) + ", " + std::to_string(m_acceleration.y) + ", " + std::to_string(m_acceleration.z) + ")");
    return QString::fromStdString(str);
}
QString Player::lookAsQString() const {
    std::string str("( " + std::to_string(m_forward.x) + ", " + std::to_string(m_forward.y) + ", " + std::to_string(m_forward.z) + ")");
    return QString::fromStdString(str);
}
