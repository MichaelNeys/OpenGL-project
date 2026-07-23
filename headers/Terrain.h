#pragma once
#include <glad/glad.h>
#include <vector>

class Shader;

class Terrain {
public:
    Terrain();
    ~Terrain();
    
    void Draw(Shader& shader);

private:
    void generate();

    unsigned int VAO = 0, VBO = 0;
    unsigned int vertexCount = 0;
};