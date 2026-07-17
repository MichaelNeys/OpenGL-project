#include "Track/TrackRenderer.h"
#include <cstdlib>
#include <ctime>

TrackRenderer::TrackRenderer() {}

TrackRenderer::~TrackRenderer() {
    if (m_trackLineVAO != 0) {
        glDeleteVertexArrays(1, &m_trackLineVAO);
        glDeleteBuffers(1, &m_trackLineVBO);
    }
}

void TrackRenderer::initialize(Mesh* pollenMesh) {
    m_pollenMesh = pollenMesh;
}

void TrackRenderer::updateTrackData(const BezierPath* currentTrack) {
    if (!currentTrack) return;

    // 1. Update de visuele hulplijn via Forward Differencing
    std::vector<glm::vec3> linePoints = currentTrack->generateVisualPath(100);
    m_trackLineVertexCount = static_cast<unsigned int>(linePoints.size());

    // Genereer OpenGL buffers als deze nog niet bestaan
    if (m_trackLineVAO == 0) {
        glGenVertexArrays(1, &m_trackLineVAO);
        glGenBuffers(1, &m_trackLineVBO);
    }

    glBindVertexArray(m_trackLineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_trackLineVBO);
    
    // Stuur de berekende forward differencing punten naar de GPU
    glBufferData(GL_ARRAY_BUFFER, linePoints.size() * sizeof(glm::vec3), linePoints.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // 2. Update de pollen-puzzelstukjes langs de nieuwe Forward Differencing route
    srand(static_cast<unsigned int>(time(NULL)));
    m_pollenMatrices.clear();

    for (size_t i = 0; i < linePoints.size(); i++) {
        glm::vec3 basePos = linePoints[i];
        const float radius = 0.025f;
        float randX = ((rand() % 1000) / 1000.0f - 0.5f) * 2.0f * radius;
        float randY = ((rand() % 1000) / 1000.0f - 0.5f) * 2.0f * radius;
        float randZ = ((rand() % 1000) / 1000.0f - 0.5f) * 2.0f * radius;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, basePos + glm::vec3(randX, randY, randZ));
        model = glm::rotate(model, basePos.x * 15.0f, glm::vec3(1.0f, 0.5f, 0.2f));
        model = glm::rotate(model, basePos.z * 10.0f, glm::vec3(0.2f, 1.0f, 0.5f));
        model = glm::scale(model, glm::vec3(0.005f));
        m_pollenMatrices.push_back(model);
    }
}

void TrackRenderer::Draw(Shader& lightingShader, Shader& lampShader, const glm::mat4& view, const glm::mat4& projection) {
    // Teken Pollen
    if (m_pollenMesh) {
        lightingShader.use();
        lightingShader.setMat4("view", view);
        lightingShader.setMat4("projection", projection);
        lightingShader.setBool("hasDiffuseTexture", false);
        lightingShader.setVec3("fallbackColor", glm::vec3(1.0f, 0.8f, 0.2f));

        glDisable(GL_CULL_FACE);
        for (const auto& model : m_pollenMatrices) {
            lightingShader.setMat4("model", model);
            m_pollenMesh->Draw();
        }
        glEnable(GL_CULL_FACE);
    }

    // Teken Forward Differencing Lijn
    if (m_trackLineVAO != 0) {
        glDisable(GL_CULL_FACE);
        lampShader.use();
        lampShader.setMat4("model", glm::mat4(1.0f));
        lampShader.setMat4("view", view);
        lampShader.setMat4("projection", projection);
        lampShader.setBool("hasDiffuseTexture", false);
        lampShader.setVec3("fallbackColor", glm::vec3(0.0f, 1.0f, 0.0f));
        
        glLineWidth(4.0f);
        glBindVertexArray(m_trackLineVAO);
        glDrawArrays(GL_LINE_STRIP, 0, m_trackLineVertexCount);
        glBindVertexArray(0);
        glLineWidth(1.0f);
        glEnable(GL_CULL_FACE);
    }
}