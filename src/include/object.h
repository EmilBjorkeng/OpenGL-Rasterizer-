#ifndef __OBJ_H__
#define __OBJ_H__

#include "shader.h"

#include <glm/glm.hpp>
#include <vector>

struct Material {
    std::string name;

    //glm::vec3 ambientColor;     // Ka - Ambient color
    glm::vec3 diffuseColor;     // Kd - Diffuse color
    //glm::vec3 specularColor;    // Ks - Specular color
    //glm::vec3 EmissiveColor;    // Ke - Emissive color
    //float shininess;            // Ns - Specular exponent
    //float refraction;           // Ni - Optical density
    float opacity;              // d or Tr - Transparency (1.0 = opaque)

    unsigned int diffuseTexture;    // map_Kd - Diffuse texture
    //unsigned int specularTexture;   // map_Ks - Specular texture
    //unsigned int normalMap;         // bump or map_Bump - Normal map
};

struct Object {
    std::vector<float> vertices;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);

    std::vector<unsigned int> textures;
    const Shader *shader;

    unsigned int VAO;
    unsigned int VBO;

    Object(const char* path, const Shader* shader);

    ~Object() {
        if (VBO) glDeleteBuffers(1, &VBO);
        if (VAO) glDeleteVertexArrays(1, &VAO);
    }

    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;
};

struct Vertex {
    glm::vec3 point;
    glm::vec2 texture;
    glm::vec3 normal;
};

std::vector<float> LoadObjFile(const char* path, std::vector<unsigned int> &textures);
Material loadMaterial(const char* path, std::string currentMaterial);
std::vector<float> MakeTriangle(std::vector<Vertex>& vertices, const Material &material);

#endif