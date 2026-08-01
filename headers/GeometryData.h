#pragma once
#include <array>
#include <glm/glm.hpp>

namespace Geometry {
    // CUBE VERTICES
    inline constexpr std::array<float, 216> cubeVertices = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    // CROSSHAIR VERTICES
    inline constexpr std::array<float, 32> crosshairVertices = {
        -0.03f,  0.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
         0.03f,  0.0f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
         0.0f, -0.04f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
         0.0f,  0.04f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f
    };

    // SKYBOX VERTICES
    inline constexpr std::array<float, 108> skyboxVertices = {
        -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,  -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,   1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f
    };

    // POINT LIGHT POSITIONS
    inline const std::array<glm::vec3, 18> pointLightPositions = {
        // Straat achter
        glm::vec3(-11.0f, -0.5f, -19.0f),
        glm::vec3( -6.0f, -0.5f, -19.0f),
        glm::vec3( -2.0f, -0.5f, -19.0f),
        glm::vec3(  2.0f, -0.5f, -19.0f),
        glm::vec3(  6.0f, -0.5f, -19.0f),
        glm::vec3( 11.0f, -0.5f, -19.0f),

        // Station
        glm::vec3(  1.0f,  0.5f, -15.0f),
        glm::vec3( 13.0f,  0.5f, -10.0f),

        // Klokken
        glm::vec3(-12.0f,  4.5f, -11.0f),
        glm::vec3( 15.0f,  4.5f, -12.0f),

        // Straat voor
        glm::vec3(-11.0f, -0.5f,  -6.0f),
        glm::vec3( -6.0f, -0.5f,  -6.0f),
        glm::vec3( -2.0f, -0.5f,  -6.0f),
        glm::vec3(  2.0f, -0.5f,  -6.0f),
        glm::vec3(  6.0f, -0.5f,  -6.0f),
        glm::vec3( 11.0f, -0.5f,  -6.0f),

        // Tussen gebouwen
        glm::vec3(-14.0f,  0.5f, -10.0f),
        glm::vec3(-14.0f,  0.5f, -14.0f)
    };

    // aantal lichten
    inline constexpr size_t numPointLights = pointLightPositions.size();

    // SCREEN QUAD VERTICES
    inline constexpr std::array<float, 24> quadVertices = {
        -1.0f,  1.0f,       0.0f, 1.0f,
        -1.0f, -1.0f,       0.0f, 0.0f,
         1.0f, -1.0f,       1.0f, 0.0f,

        -1.0f,  1.0f,       0.0f, 1.0f,
         1.0f, -1.0f,       1.0f, 0.0f,
         1.0f,  1.0f,       1.0f, 1.0f
    };

    // layout van een zijkant face
    struct BlockFaceData {
        glm::vec3 normal;
        // 6 vertices per face: [offX, offZ, u, v]
        std::array<std::array<float, 4>, 6> vertices;
    };

    enum FaceDirection { 
        FACE_PX = 0, 
        FACE_NX = 1, 
        FACE_PZ = 2, 
        FACE_NZ = 3, 
        FACE_TOP = 4 
    };

    // vlakken voor terrein
    inline constexpr BlockFaceData blockFaces[5] = {
        // 0: +X (Rechts)
        { glm::vec3(1.0f, 0.0f, 0.0f), {{
            { 0.5f,  0.5f, 0.0f, 0.0f}, { 0.5f, -0.5f, 1.0f, 0.0f}, { 0.5f, -0.5f, 1.0f, 1.0f},
            { 0.5f,  0.5f, 0.0f, 0.0f}, { 0.5f, -0.5f, 1.0f, 1.0f}, { 0.5f,  0.5f, 0.0f, 1.0f}
        }}},
        // 1: -X (Links)
        { glm::vec3(-1.0f, 0.0f, 0.0f), {{
            {-0.5f, -0.5f, 0.0f, 0.0f}, {-0.5f,  0.5f, 1.0f, 0.0f}, {-0.5f,  0.5f, 1.0f, 1.0f},
            {-0.5f, -0.5f, 0.0f, 0.0f}, {-0.5f,  0.5f, 1.0f, 1.0f}, {-0.5f, -0.5f, 0.0f, 1.0f}
        }}},
        // 2: +Z (Voor)
        { glm::vec3(0.0f, 0.0f, 1.0f), {{
            {-0.5f,  0.5f, 0.0f, 0.0f}, { 0.5f,  0.5f, 1.0f, 0.0f}, { 0.5f,  0.5f, 1.0f, 1.0f},
            {-0.5f,  0.5f, 0.0f, 0.0f}, { 0.5f,  0.5f, 1.0f, 1.0f}, {-0.5f,  0.5f, 0.0f, 1.0f}
        }}},
        // 3: -Z (Achter)
        { glm::vec3(0.0f, 0.0f, -1.0f), {{
            { 0.5f, -0.5f, 0.0f, 0.0f}, {-0.5f, -0.5f, 1.0f, 0.0f}, {-0.5f, -0.5f, 1.0f, 1.0f},
            { 0.5f, -0.5f, 0.0f, 0.0f}, {-0.5f, -0.5f, 1.0f, 1.0f}, { 0.5f, -0.5f, 0.0f, 1.0f}
        }}},
        // 4: TOP (Bovenkant)
        { glm::vec3(0.0f, 1.0f, 0.0f), {{
            {-0.5f, -0.5f, 0.0f, 0.0f}, { 0.5f, -0.5f, 1.0f, 0.0f}, { 0.5f,  0.5f, 1.0f, 1.0f},
            {-0.5f, -0.5f, 0.0f, 0.0f}, { 0.5f,  0.5f, 1.0f, 1.0f}, {-0.5f,  0.5f, 0.0f, 1.0f}
        }}}
    };
}