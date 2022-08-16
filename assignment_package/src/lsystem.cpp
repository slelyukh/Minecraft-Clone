#include <lsystem.h>

Lsystem::Lsystem() :
    Lsystem(std::string("F"))
{}

Lsystem::Lsystem(std::string init) :
    lstring(init)
{}

void Lsystem::constructLString(int iterations, float prob, float branch) {
    for (int i=0; i<iterations; i++) {
        lstring = replaceF(lstring, std::string("F"), prob, branch);
        lstring = replaceLR(lstring);
    }
}

std::string Lsystem::replaceLR(std::string str) {
    size_t start_pos = 0;
    std::string from = "R";
    std::string to = "FF";
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    start_pos = 0;
    from = "L";
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

std::string Lsystem::replaceF(std::string str, std::string from, float prob, float branch) {
    //copied from https://stackoverflow.com/questions/2896600/how-to-replace-all-occurrences-of-a-character-in-string
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        float p = ((float) rand() / (RAND_MAX));
        float i = ((float) rand() / (RAND_MAX));
        int index = i > branch ? 0 : std::floor(1 + ((float) rand() / (RAND_MAX)) * 2);
        std::string to = rules[index];
        if (p > prob) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
        } else {
            start_pos++;
        }
    }
    return str;
}

Position::Position(glm::vec3 p, float angle) :
    p(p), forward(0,0,1), right(1,0,0), up(0,1,0)
{
    rotate(angle);
}

void Position::rotate(float angle) {
    float rad = glm::radians(angle);
    forward = glm::vec3(glm::rotate(glm::mat4(), rad, glm::vec3(0,1,0)) * glm::vec4(forward, 0.f));
    right = glm::vec3(glm::rotate(glm::mat4(), rad, glm::vec3(0,1,0)) * glm::vec4(right, 0.f));
    up = glm::vec3(glm::rotate(glm::mat4(), rad, glm::vec3(0,1,0)) * glm::vec4(up, 0.f));
}

glm::ivec2 Position::move(float dist) {
    p += dist * forward;
    glm::vec3 rounded = glm::round(p);
    return glm::ivec2(rounded.x, rounded.z);
}

LineSegment::LineSegment(glm::vec2 end1, glm::vec2 end2)
    : end1(end1), end2(end2), dy(end2.y-end1.y), dx(end2.x-end1.x)
{}

LineSegment::LineSegment()
    : end1(), end2(), dy(0), dx(0)
{}

void LineSegment::printSeg() {
    std::cout << glm::to_string(end1) << ", " << glm::to_string(end2) << std::endl;
}

bool LineSegment::getIntersection(int y, float* x) {
    if (dx == 0) { //vertical line
        *x = end1.x;
        return true;
    }
    if (dy == 0) { //horizontal line
        return false;
    }
    float slope = dy / dx;
    float intersect = (y + slope*end1.x - end1.y) / slope;
    *x = intersect;
    return true;
}

//bool LineSegment::checkY(int y) {
//    return (y >= std::min(this->end1[1],this->end2[1]) && y <= std::max(this->end1[1],this->end2[1]));
//}
