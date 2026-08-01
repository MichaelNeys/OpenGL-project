#include "Terrain.h"
#include "GeometryData.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <cmath>
#include <algorithm>
#include <iostream>

/**
 * @brief toevoegen zijde
 * 
 * @param out vector met vertexdata
 * @param cx X pos centrum
 * @param cy Y pos centrum
 * @param cz Z pos centrum
 * @param neighborY hoogte buur
 * @param dir righting face
 */
static void appendFace(std::vector<float>& out, float cx, float cy, float cz, float neighborY, Geometry::FaceDirection dir) {
    constexpr float h = 0.5f;
    const float top = cy + h;
    const float bottom = (dir == Geometry::FACE_TOP) ? top : (neighborY + h);
    const auto& faceData = Geometry::blockFaces[dir];

    for (const auto& v : faceData.vertices) {
        float vx = cx + v[0];
        float vy = (v[3] == 0.0f) ? top : bottom;
        float vz = cz + v[1];

        // Positie (3 floats)
        out.push_back(vx);
        out.push_back(vy);
        out.push_back(vz);
        // Normaal (3 floats)
        out.push_back(faceData.normal.x);
        out.push_back(faceData.normal.y);
        out.push_back(faceData.normal.z);
        // TexCoords (2 floats)
        out.push_back(v[2]);
        out.push_back(v[3]);
    }
}

/**
 * @brief constructor
 * 
 * call generate() to build terrain geometry
 */
Terrain::Terrain() {
    generate();
}

/**
 * @brief destructor
 * 
 * clear allocated OpenGL buffers
 */
Terrain::~Terrain() {
    if (vertexCount > 0) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
}

/**
 * @brief generates geometry of terrain
 * 
 * build heightmap, sinus voor hills, plat voor de rest
 * enkel zichtbare bovenvlakken en zijkanten
 * naar VBO op GPU
 */
void Terrain::generate() {
    constexpr int gx0 = -65, gx1 = 65;
    constexpr int gz0 = -80, gz1 = 40;
    constexpr float blockScale = 0.50f;

    constexpr float villageX0 = -20.0f, villageX1 = 20.0f;
    constexpr float villageZ0 = -25.5f, villageZ1 =  5.0f;

    const int cols = gx1 - gx0 + 1;
    const int rows = gz1 - gz0 + 1;

    // 1. Bouw eerst een heightmap zodat we buren kunnen opzoeken
    std::vector<float> heightMap(cols * rows);

    for (int gx = gx0; gx <= gx1; ++gx) {
        for (int gz = gz0; gz <= gz1; ++gz) {
            const float wx = gx * blockScale;
            const float wz = gz * blockScale;

            const bool insideVillage = (wx >= villageX0 && wx <= villageX1 && wz >= villageZ0 && wz <= villageZ1);
            int top;
            if (insideVillage) {
                top = -3;
            } else {
                const float fx = gx * 0.18f;
                const float fz = gz * 0.16f;
                float wobble = 1.2f * std::sin(fx) * std::cos(fz) + 0.6f * std::sin(fx * 2.2f + fz * 1.05f);
                wobble += 0.4f * std::sin(static_cast<float>(gx + gz) * 0.28f);
                int dh = static_cast<int>(std::round(wobble));
                dh = std::clamp(dh, -2, 2);
                top = -3 + dh;
            }

            const int col = gx - gx0;
            const int row = gz - gz0;
            heightMap[col * rows + row] = static_cast<float>(top) * blockScale;
        }
    }

    // Helper: hoogte van buur ophalen
    auto getHeight = [&](int gx, int gz) -> float {
        if (gx < gx0 || gx > gx1 || gz < gz0 || gz > gz1)
            return heightMap[(std::clamp(gx, gx0, gx1) - gx0) * rows + (std::clamp(gz, gz0, gz1) - gz0)];
        return heightMap[(gx - gx0) * rows + (gz - gz0)];
    };

    // 2. bovenkant altijd, zijkanten alleen bij hoogteverschil
    std::vector<float> groundData;
    groundData.reserve(cols * rows * 6 * 8);

    for (int gx = gx0; gx <= gx1; ++gx) {
        for (int gz = gz0; gz <= gz1; ++gz) {
            const float wx = gx * blockScale;
            const float wz = gz * blockScale;
            const float cy = getHeight(gx, gz);

            appendFace(groundData, wx, cy, wz, 0.0f, Geometry::FACE_TOP);

            // Teken een zijkant alleen als de buur lager is
            const float hPX = getHeight(gx + 1, gz);
            const float hNX = getHeight(gx - 1, gz);
            const float hPZ = getHeight(gx, gz + 1);
            const float hNZ = getHeight(gx, gz - 1);

            if (hPX < cy) appendFace(groundData, wx, cy, wz, hPX, Geometry::FACE_PX);
            if (hNX < cy) appendFace(groundData, wx, cy, wz, hNX, Geometry::FACE_NX);
            if (hPZ < cy) appendFace(groundData, wx, cy, wz, hPZ, Geometry::FACE_PZ);
            if (hNZ < cy) appendFace(groundData, wx, cy, wz, hNZ, Geometry::FACE_NZ);
        }
    }

    vertexCount = static_cast<unsigned int>(groundData.size() / 8);
    if (vertexCount == 0) return;

    // 3. data naar GPU
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(groundData.size() * sizeof(float)), groundData.data(), GL_STATIC_DRAW);

    // Positie
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), static_cast<void*>(0));
    glEnableVertexAttribArray(0);

    // Normaal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Texcoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

/**
 * @brief render terrein op scherm
 * 
 * @param shader LightingShader voor materiaal en schaduwen
 */
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