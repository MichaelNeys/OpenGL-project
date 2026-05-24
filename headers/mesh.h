#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "shader.h"

class Mesh {
public:
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

    unsigned int VAO, VBO;
    unsigned int vertexCount, stride;

    const std::vector<Vertex>& getVertices() const { return _vertices; }
    const std::vector<unsigned int>& getIndices() const { return _indices; }

    Mesh(float* vertices, unsigned int vertexCount, bool hasNormals);
    void Draw();
    void Delete();

    // Nieuwe constructor
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    void Draw(Shader& shader);
    void DeleteModel();

private:
    unsigned int EBO;
    std::vector<Vertex> _vertices;
    std::vector<unsigned int> _indices;
    std::vector<Texture> _textures;
    void setupModelMesh();
};
