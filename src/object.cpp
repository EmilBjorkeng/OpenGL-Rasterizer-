#include "object.h"
#include "STB/stb_image.h"
#include <algorithm>

Object::Object(const char* path, const Shader *shader) {
    this->vertices = LoadObjFile(path, this->textures);
    this->shader = shader;

    // Generate VAO and VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    // Texture Coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 12*sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
    // Diffuse Color attribute
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 12*sizeof(float), (void*)(8*sizeof(float)));
    glEnableVertexAttribArray(3);
    // Opacity attribute
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 12*sizeof(float), (void*)(11*sizeof(float)));
    glEnableVertexAttribArray(4);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

std::vector<float> LoadObjFile(const char* path, std::vector<unsigned int> &textures) {
    std::vector<float> result;

    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "Failed to open OBJ file: " << path << "\n";
        return result;
    }
    std::string line;

    std::vector<glm::vec3> vertex_list;
    std::vector<glm::vec2> texture_list;
    std::vector<glm::vec3> normal_list;

    std::vector<Material> materials;
    std::string currentMaterial;
    std::string materialLib;

    int lineNumber = 0;
    while (std::getline(in, line)) {
        ++lineNumber;

        std::istringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "v") {
            glm::vec3 vert;
            ss >> vert.x >> vert.y >> vert.z;
            vertex_list.push_back(vert);
        }
        else if (type == "vt") {
            glm::vec2 text;
            ss >> text.x >> text.y;
            texture_list.push_back(text);
        }
        else if (type == "vn") {
            glm::vec3 norm;
            ss >> norm.x >> norm.y >> norm.z;
            normal_list.push_back(norm);
        }
        else if (type == "f") {
            std::vector<Vertex> face_vertices;
            std::string vert;
            while (ss >> vert) {
                int v = 0, vt = 0, vn = 0;

                std::vector<std::string> parts;
                std::stringstream tokenStream(vert);
                std::string token;

                // Split into vertecies, normals, texture using the '/'
                while (std::getline(tokenStream, token, '/')) {
                    parts.push_back(token);
                }

                try {
                    if (parts.size() >= 1 && !parts[0].empty())
                        v = std::stoi(parts[0]);
                    if (parts.size() >= 2 && !parts[1].empty())
                        vt = std::stoi(parts[1]);
                    if (parts.size() >= 3 && !parts[2].empty())
                        vn = std::stoi(parts[2]);
                }
                catch (const std::exception& err) {
                    std::cerr << "Invalid face format: " << vert
                        << " (" << err.what() << ") at line " << lineNumber
                        << " in file: " << path << std::endl;
                    continue;
                }

                Vertex vertex;
                // Vertex point data
                vertex.point = vertex_list[v - 1];

                // Vertex texture data
                if (vt > 0 && vt <= (int)texture_list.size())
                    vertex.texture = texture_list[vt - 1];
                else
                    vertex.texture = glm::vec2(0.0f, 0.0f);

                // Vertex normal data
                if (vn > 0 && vn <= (int)normal_list.size())
                    vertex.normal = normal_list[vn - 1];
                else {
                    // Compute the face normal if vertex normal is missing
                    glm::vec3 edge1 = vertex_list[v - 1] - vertex_list[0];
                    glm::vec3 edge2 = vertex_list[(v % vertex_list.size())] - vertex_list[0];
                    vertex.normal = glm::normalize(glm::cross(edge1, edge2));
                }

                face_vertices.push_back(vertex);
            }

            // Get the MTL file path
            std::string objDir(path);
            size_t lastSlash = objDir.find_last_of("/\\");
            objDir = (lastSlash != std::string::npos) ? objDir.substr(0, lastSlash + 1) : "";
            std::string mtlPath = objDir + materialLib;

            // Check if material is already loaded
            Material mat;
            bool foundMaterial = false;
            for (auto& m : materials) {
                if (m.name == currentMaterial) {
                    mat = m;
                    foundMaterial = true;
                    break;
                }
            }
            if (!foundMaterial) {
                // Load material if not found
                mat = loadMaterial(mtlPath.c_str(), currentMaterial);
                materials.push_back(mat);
                if (mat.diffuseTexture)
                    textures.push_back(mat.diffuseTexture);
            }

            std::vector<float> triangles = MakeTriangle(face_vertices, mat);
            result.insert(result.end(), triangles.begin(), triangles.end());
        }
        else if (type == "mtllib") {
            ss >> materialLib;
        }
        else if (type == "usemtl") {
            ss >> currentMaterial;
        }
    }

    return result;
}

unsigned int loadImage(const char* path) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (!data) {
        std::cout << "Failed to load texture: " << path << std::endl;
        return 0;
    }

    GLenum format;
    GLenum internalFormat;

    if (nrChannels == 1)
        format = internalFormat = GL_RED;
    else if (nrChannels == 3) {
        format = GL_RGB;
        internalFormat = GL_SRGB;
    } else if (nrChannels == 4) {
        format = GL_RGBA;
        internalFormat = GL_SRGB_ALPHA;
    } else {
        std::cout << "Unsupported number of channels: " << nrChannels << std::endl;
        stbi_image_free(data);
        return 0;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    return texture;
}

Material loadMaterial(const char* path, std::string currentMaterial) {
    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "Failed to open MTL file: " << path << "\n";
        return {};
    }

    Material result;
    // Default values
    result.name = currentMaterial;
    result.diffuseColor = glm::vec3(0.8f, 0.8f, 0.8f);
    result.opacity = 1.0f;
    result.diffuseTexture = 0;

    // Get directory for local paths
    std::string directory = path;
    size_t lastSlash = directory.find_last_of("/\\");
    if (lastSlash != std::string::npos)
        directory = directory.substr(0, lastSlash + 1);
    else
        directory = "";

    std::string line;
    bool skipping = false;
    while (std::getline(in, line)) {
        std::istringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "newmtl") {
            skipping = false;
            std::string name;
            ss >> name;
            if (name != currentMaterial) {
                // Skip materials until we find the one we want
                skipping = true;
                continue;
            }
            result.name = name;
        } else if (skipping) {
            continue;
        // Color
        } else if (type == "Kd") {
            ss >> result.diffuseColor.x >> result.diffuseColor.y >> result.diffuseColor.z;
        } else if (type == "d") {
            ss >> result.opacity;
        } else if (type == "Tr") {
            float  transparency;
            ss >> transparency;
            result.opacity = 1.0f - transparency;
        // Texture
        } else if (type == "map_Kd") {
            std::string diffuseTexturePath;
            ss >> diffuseTexturePath;
            result.diffuseTexture = loadImage((directory + diffuseTexturePath).c_str());
        }
    }

    return result;
}

// Merge Verticies, normal and texture data and return it as a OpenGL compatable format
// Triangles made using Ear clipping algorithm
// Also append any material data
std::vector<float> MakeTriangle(std::vector<Vertex>& vertices, const Material &material) {
    std::vector<float> result;
    int n = vertices.size();
    if (n < 3) return result;

    // Create Plane for the face
    glm::vec3 edge1 = vertices[1].point - vertices[0].point;
    glm::vec3 edge2 = vertices[2].point - vertices[0].point;
    glm::vec3 N = glm::normalize(glm::cross(edge1, edge2));
    // Use U and V basis for the plane
    glm::vec3 U = glm::normalize(edge1);
    glm::vec3 V = glm::cross(N, U);

    // Project vertices onto the 2D plane
    std::vector<glm::vec2> projected;
    projected.reserve(vertices.size());
    for (const auto &p : vertices) {
        glm::vec3 vec = p.point - vertices[0].point;
        projected.emplace_back(glm::dot(vec, U), glm::dot(vec, V));
    }

    // Check orientation
    float totalArea = 0.0f;
    for (int i = 0; i < (int)projected.size(); i++) {
        int j = (i + 1) % projected.size();
        totalArea += (projected[j].x - projected[i].x) * (projected[j].y + projected[i].y);
    }
    bool ccw = (totalArea < 0.0f);

    // Ensure orientation is Counter Clockwise
    if (!ccw) {
        std::reverse(vertices.begin(), vertices.end());
        projected.clear();
        projected.reserve(vertices.size());

        // recompute projection for reversed vertex order
        for (const auto &p : vertices) {
            glm::vec3 vec = p.point - vertices[0].point;
            projected.emplace_back(glm::dot(vec, U), glm::dot(vec, V));
        }
    }

    auto push_to_result = [&result, &material](Vertex &vertex) {
        result.push_back(vertex.point.x);
        result.push_back(vertex.point.y);
        result.push_back(vertex.point.z);
        result.push_back(vertex.normal.x);
        result.push_back(vertex.normal.y);
        result.push_back(vertex.normal.z);
        result.push_back(vertex.texture.x);
        result.push_back(vertex.texture.y);
        result.push_back(material.diffuseColor.x);
        result.push_back(material.diffuseColor.y);
        result.push_back(material.diffuseColor.z);
        result.push_back(material.opacity);
    };

    auto pointInTriangle = [&](const glm::vec2& P, const glm::vec2& A, const glm::vec2& B, const glm::vec2& C)
    {
        float c1 = (B.x - A.x) * (P.y - A.y) - (B.y - A.y) * (P.x - A.x);
        float c2 = (C.x - B.x) * (P.y - B.y) - (C.y - B.y) * (P.x - B.x);
        float c3 = (A.x - C.x) * (P.y - C.y) - (A.y - C.y) * (P.x - C.x);

        bool hasNeg = (c1 < 0) || (c2 < 0) || (c3 < 0);
        bool hasPos = (c1 > 0) || (c2 > 0) || (c3 > 0);

        return !(hasNeg && hasPos);
    };

    std::vector<int> indices(n);
    for (int i = 0; i < n; ++i) indices[i] = i;

    while (indices.size() > 3) {
        bool earFound = false;

        for (int i = 0; i < (int)indices.size(); ++i) {
            int prev = indices[(i + indices.size() - 1) % indices.size()];
            int curr = indices[i];
            int next = indices[(i + 1) % indices.size()];

            glm::vec2 &a = projected[prev];
            glm::vec2 &b = projected[curr];
            glm::vec2 &c = projected[next];

            // Convex check
            float check = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
            if (check <= 0.0f) continue;

            // Check for vertices inside triangle
            bool hasPointInside = false;
            for (int j = 0; j < (int)indices.size(); ++j) {
                int vi = indices[j];
                if (vi == prev || vi == curr || vi == next) continue;

                if (pointInTriangle(projected[vi], a, b, c)) {
                    hasPointInside = true;
                    break;
                }
            }
            if (hasPointInside) continue;

            push_to_result(vertices[prev]);
            push_to_result(vertices[curr]);
            push_to_result(vertices[next]);

            indices.erase(indices.begin() + i);
            earFound = true;
            break;
        }

        if (!earFound) break; // No valid ear
    }

    // Final triangle
if (indices.size() == 3) {
    push_to_result(vertices[indices[0]]);
    push_to_result(vertices[indices[1]]);
    push_to_result(vertices[indices[2]]);
}

    return result;
}