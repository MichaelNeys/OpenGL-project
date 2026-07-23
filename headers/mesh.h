#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "shader.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    unsigned int id;
    std::string  type;
    std::string  path;
};

class Mesh {
public:
    // Constructor 1: Simpele float array
    Mesh(const float* vertices, unsigned int vertexCount, bool hasNormals);
    
    // Constructor 2: Model mesh met struct Vertex/Indices/Textures
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    ~Mesh();

    // RULE OF FIVE: Voorkomt dubbele deletes bij gebruik in std::vector
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void Draw();
    void Draw(Shader& shader);

private:
    void cleanup();

    unsigned int vertexCount = 0;
    int stride = 3;

    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;

    std::vector<Vertex> _vertices;
    std::vector<unsigned int> _indices;
    std::vector<Texture> _textures;
};