#ifndef __LIGHT_H__
#define __LIGHT_H__

#include <glm/glm.hpp>

struct Light {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};

#endif