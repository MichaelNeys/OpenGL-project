#include "Terrain.h"
#include "GeometryData.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <cmath>
#include <algorithm>
#include <iostream>

// Bovenkant van een blok — altijd tekenen
static void appendTopFace(std::vector<float>& out, float cx, float cy, float cz) {
    const float h = 0.5f;
    float face[] = {
        cx-h, cy+h, cz-h, 0,1,0, 0,0,
        cx+h, cy+h, cz-h, 0,1,0, 1,0,
        cx+h, cy+h, cz+h, 0,1,0, 1,1,

        cx-h, cy+h, cz-h, 0,1,0, 0,0,
        cx+h, cy+h, cz+h, 0,1,0, 1,1,
        cx-h, cy+h, cz+h, 0,1,0, 0,1,
    };
    for (float f : face) out.push_back(f);
}

// Zijkant richting +X (rechts)
static void appendFacePX(std::vector<float>& out, float cx, float cy, float cz, float neighborY) {
    const float h = 0.5f;
    float top = cy + h;
    float bottom = neighborY + h;
    float face[] = {
        cx+h, top, cz+h, 1,0,0, 0,0,
        cx+h, top, cz-h, 1,0,0, 1,0,
        cx+h, bottom, cz-h, 1,0,0, 1,1,

        cx+h, top, cz+h, 1,0,0, 0,0,
        cx+h, bottom, cz-h, 1,0,0, 1,1,
        cx+h, bottom, cz+h, 1,0,0, 0,1,
    };
    for (float f : face) out.push_back(f);
}

// Zijkant richting -X (links)
static void appendFaceNX(std::vector<float>& out, float cx, float cy, float cz, float neighborY) {
    const float h = 0.5f;
    float top = cy + h;
    float bottom = neighborY + h;
    float face[] = {
        cx-h, top, cz-h, -1,0,0, 0,0,
        cx-h, top, cz+h, -1,0,0, 1,0,
        cx-h, bottom, cz+h, -1,0,0, 1,1,

        cx-h, top, cz-h, -1,0,0, 0,0,
        cx-h, bottom, cz+h, -1,0,0, 1,1,
        cx-h, bottom, cz-h, -1,0,0, 0,1,
    };
    for (float f : face) out.push_back(f);
}

// Zijkant richting +Z (voor)
static void appendFacePZ(std::vector<float>& out, float cx, float cy, float cz, float neighborY) {
    const float h = 0.5f;
    float top = cy + h;
    float bottom = neighborY + h;
    float face[] = {
        cx-h, top, cz+h, 0,0,1, 0,0,
        cx+h, top, cz+h, 0,0,1, 1,0,
        cx+h, bottom, cz+h, 0,0,1, 1,1,

        cx-h, top, cz+h, 0,0,1, 0,0,
        cx+h, bottom, cz+h, 0,0,1, 1,1,
        cx-h, bottom, cz+h, 0,0,1, 0,1,
    };
    for (float f : face) out.push_back(f);
}

// Zijkant richting -Z (achter)
static void appendFaceNZ(std::vector<float>& out, float cx, float cy, float cz, float neighborY) {
    const float h = 0.5f;
    float top = cy + h;
    float bottom = neighborY + h;
    float face[] = {
        cx+h, top, cz-h, 0,0,-1, 0,0,
        cx-h, top, cz-h, 0,0,-1, 1,0,
        cx-h, bottom, cz-h, 0,0,-1, 1,1,

        cx+h, top, cz-h, 0,0,-1, 0,0,
        cx-h, bottom, cz-h, 0,0,-1, 1,1,
        cx+h, bottom, cz-h, 0,0,-1, 0,1,
    };
    for (float f : face) out.push_back(f);
}

Terrain::Terrain() {
    generate();
}

Terrain::~Terrain() {
    if (vertexCount > 0) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
}

void Terrain::generate() {
    const int gx0 = -65, gx1 = 65;
    const int gz0 = -80, gz1 = 40;
    const float blockScale = 0.50f;

    const float villageX0 = -20.0f, villageX1 = 20.0f;
    const float villageZ0 = -25.5f, villageZ1 =  5.0f;

    int cols = gx1 - gx0 + 1;
    int rows = gz1 - gz0 + 1;

    // Bouw eerst een heightmap zodat we buren kunnen opzoeken
    std::vector<float> heightMap(cols * rows);

    for (int gx = gx0; gx <= gx1; ++gx) {
        for (int gz = gz0; gz <= gz1; ++gz) {
            float wx = gx * blockScale;
            float wz = gz * blockScale;

            bool insideVillage = (wx >= villageX0 && wx <= villageX1 && wz >= villageZ0 && wz <= villageZ1);
            int top;
            if (insideVillage) {
                top = -3;
            } else {
                float fx = gx * 0.18f;
                float fz = gz * 0.16f;
                float wobble = 1.2f * std::sin(fx) * std::cos(fz) + 0.6f * std::sin(fx * 2.2f + fz * 1.05f);
                wobble += 0.4f * std::sin(static_cast<float>(gx + gz) * 0.28f);
                int dh = static_cast<int>(std::round(wobble));
                dh = std::clamp(dh, -2, 2);
                top = -3 + dh;
            }

            int col = gx - gx0;
            int row = gz - gz0;
            heightMap[col * rows + row] = (float)top * blockScale;
        }
    }

    // Helper: hoogte van buur ophalen
    auto getHeight = [&](int gx, int gz) -> float {
        if (gx < gx0 || gx > gx1 || gz < gz0 || gz > gz1)
            return heightMap[(std::clamp(gx, gx0, gx1) - gx0) * rows + (std::clamp(gz, gz0, gz1) - gz0)];
        return heightMap[(gx - gx0) * rows + (gz - gz0)];
    };

    // Genereer geometry — bovenkant altijd, zijkanten alleen bij hoogteverschil
    std::vector<float> groundData;
    groundData.reserve(cols * rows * 48);

    for (int gx = gx0; gx <= gx1; ++gx) {
        for (int gz = gz0; gz <= gz1; ++gz) {
            float wx = gx * blockScale;
            float wz = gz * blockScale;
            float cy = getHeight(gx, gz);

            appendTopFace(groundData, wx, cy, wz);

            // Teken een zijkant alleen als de buur lager is
            float hPX = getHeight(gx + 1, gz);
            float hNX = getHeight(gx - 1, gz);
            float hPZ = getHeight(gx, gz + 1);
            float hNZ = getHeight(gx, gz - 1);

            if (hPX < cy) appendFacePX(groundData, wx, cy, wz, hPX);
            if (hNX < cy) appendFaceNX(groundData, wx, cy, wz, hNX);
            if (hPZ < cy) appendFacePZ(groundData, wx, cy, wz, hPZ);
            if (hNZ < cy) appendFaceNZ(groundData, wx, cy, wz, hNZ);
        }
    }

    vertexCount = static_cast<unsigned int>(groundData.size() / 8);
    if (vertexCount == 0) return;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(groundData.size() * sizeof(float)), groundData.data(), GL_STATIC_DRAW);

    // Positie
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normaal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Texcoords
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
