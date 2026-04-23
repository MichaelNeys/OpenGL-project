#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "shader.h"

class Mesh {
public:
    unsigned int VAO, VBO;
    unsigned int vertexCount, stride;

    // Bestaande constructor (voor kubus/lamp)
    Mesh(float* vertices, unsigned int vertexCount, bool hasNormals);
    void Draw();
    void Delete();

    // Assimp modellen
    struct Vertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
    };
    struct Texture {
        unsigned int id;
        std::string  type;  
        // "texture_diffuse" of "texture_normal"
        std::string  path;
    };

    // Nieuwe constructor
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    void Draw(Shader& shader);  // overload voor model-rendering
    void DeleteModel();

private:
    // alleen gebruikt door model-constructor
    unsigned int EBO;
    std::vector<Vertex>       _vertices;
    std::vector<unsigned int> _indices;
    std::vector<Texture>      _textures;
    void setupModelMesh();
};
