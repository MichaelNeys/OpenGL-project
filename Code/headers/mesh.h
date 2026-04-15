#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <vector>

class Mesh {
public:
    unsigned int VAO;

    Mesh(float* vertices, unsigned int vertexCount, bool hasNormals);
    void Draw();
    void Delete();

private:
    unsigned int VBO;
    unsigned int vertexCount;
    int stride;
};

#endif