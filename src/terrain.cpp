#include "terrain.h"
#include "GeometryData.h"
#include "shader.h"
#include <glm/glm.hpp>
#include <cmath>
#include <algorithm>
#include <iostream>

static void appendCubeAt(std::vector<float>& out, float cx, float cy, float cz) {
    const size_t n = 216;
    for (size_t i = 0; i < n; i += 6) {
        out.push_back(Geometry::cubeVertices[i + 0] + cx);
        out.push_back(Geometry::cubeVertices[i + 1] + cy);
        out.push_back(Geometry::cubeVertices[i + 2] + cz);
        out.push_back(Geometry::cubeVertices[i + 3]);
        out.push_back(Geometry::cubeVertices[i + 4]);
        out.push_back(Geometry::cubeVertices[i + 5]);
        out.push_back(0.0f); 
        out.push_back(0.0f); 
    }
}
Terrain::Terrain() {
    generate();
}

void Terrain::generate() {
    std::vector<float> groundData;

    const int gx0 = -65, gx1 = 65, gz0 = -80, gz1 = 40;
    const float blockScale = 0.50f;

    // Village grenzen in world space
    const float villageX0 = -20.0f, villageX1 = 20.0f;
    const float villageZ0 = -25.5f, villageZ1 = 5.0f;

    for (int gx = gx0; gx <= gx1; ++gx) {
        for (int gz = gz0; gz <= gz1; ++gz) {
            float wx = gx * blockScale;
            float wz = gz * blockScale;

            bool insideVillage = (wx >= villageX0 && wx <= villageX1 &&
                                  wz >= villageZ0 && wz <= villageZ1);

            int top;
            if (insideVillage) {
                top = -3;
            } else {
                float fx = gx * 0.18f, fz = gz * 0.16f;
                float wobble = 1.2f * std::sin(fx) * std::cos(fz)
                    + 0.6f * std::sin(fx * 2.2f + fz * 1.05f);
                wobble += 0.4f * std::sin(static_cast<float>(gx + gz) * 0.28f);
                int dh = static_cast<int>(std::round(wobble));
                dh = std::clamp(dh, -2, 2);
                top = -3 + dh;
            }

            appendCubeAt(groundData, wx, (float)top * blockScale, wz);
        }
    }

    vertexCount = static_cast<unsigned int>(groundData.size() / 8);

    if (vertexCount == 0) return;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(groundData.size() * sizeof(float)),
                 groundData.data(), GL_STATIC_DRAW);

    // positie
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normaal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texcoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Terrain::Draw(Shader& shader) {
    if (vertexCount == 0) return;

    glDisable(GL_CULL_FACE);
    shader.setBool("hasDiffuseTexture", false);
    shader.setVec3("flatAlbedo", glm::vec3(0.25f, 0.45f, 0.12f));
    shader.setMat4("model", glm::mat4(1.0f));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexCount));
    glBindVertexArray(0);
    glEnable(GL_CULL_FACE);
}

void Terrain::Delete() {
    if (vertexCount > 0) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
}