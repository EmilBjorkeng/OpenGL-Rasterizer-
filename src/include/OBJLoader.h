#ifndef __OBJLOADER_H__
#define __OBJLOADER_H__

#include "Material.h"
#include <vector>
#include <string>

struct Vertex {
    glm::vec3 point;
    glm::vec3 normal;
    glm::vec2 texture;
};

struct Face {
    std::vector<Vertex> vertices;
    Material material;
};

class OBJLoader {
public:
    static std::vector<Face> loadOBJ(const std::string& path);
};

#endif
