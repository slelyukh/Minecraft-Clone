#pragma once

#include <string>
#include <vector>
#include <math.h>
#include <iostream>
#include <glm_includes.h>

class Lsystem {
public:
    std::string lstring;
    std::vector<std::string> rules;

    void constructLString(int, float, float);
    std::string replaceF(std::string, std::string, float, float);
    std::string replaceLR(std::string);
    Lsystem();
    Lsystem(std::string);
};

class Position {
public:
    glm::vec3 p;
    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up;
    Position(glm::vec3, float);
    void rotate(float angle);
    glm::ivec2 move(float dist);
};

class LineSegment { //copied from my HW3
public:
    glm::vec2 end1;
    glm::vec2 end2;
    float dy;
    float dx;
    LineSegment();
    LineSegment(glm::vec2, glm::vec2);
    bool getIntersection(int y, float* x);
    void printSeg();
//    bool checkY(int);
};
