#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "../Mesh.h"
#include "../Shader.h"
#include "BezierPath.h"

class TrackRenderer {
public:
    TrackRenderer();
    ~TrackRenderer();

    void initialize(Mesh* pollenMesh);
    void updateTrackData(const BezierPath* currentTrack);
    void Draw(Shader& lightingShader, Shader& lampShader, const glm::mat4& view, const glm::mat4& projection);

private:
    Mesh* m_pollenMesh = nullptr;
    std::vector<glm::mat4> m_pollenMatrices;

    unsigned int m_trackLineVAO = 0;
    unsigned int m_trackLineVBO = 0;
    unsigned int m_trackLineVertexCount = 0;
};