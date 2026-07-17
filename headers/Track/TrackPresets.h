// TrackPresets.h
#pragma once
#include <glm/glm.hpp>
#include <vector>

namespace TrackPresets {
    const std::vector<glm::vec3> MainParcours = {
        glm::vec3(14.1f, 0.0f, -9.6f),
        glm::vec3(8.8f, 0.2f, -10.0f),
        glm::vec3(5.9f, 0.3f, -12.3f),
        glm::vec3(3.2f, 0.3f, -15.6f),
        glm::vec3(-1.4f, 0.8f, -15.11f),
        glm::vec3(-3.4f, 1.1f, -12.5f),
        glm::vec3(-6.3f, 0.1f, -11.0f),
        glm::vec3(-7.3f, 0.1f, -9.8f),
        glm::vec3(-11.4f, -0.1f, -9.8f),
        glm::vec3(-12.8f, -0.1f, -11.2f),
        glm::vec3(-12.3f, -0.3f, -15.3f),
        glm::vec3(-11.8f, -0.16f, -18.1f),
        glm::vec3(-11.3f, -0.16f, -22.3f),
        glm::vec3(-11.8f, -0.25f, -23.0f),
        glm::vec3(-12.5f, -0.25f, -22.5f),
        glm::vec3(-9.9f, 0.35f, -19.6f),
        glm::vec3(2.3f, -0.6f, -18.6f),
        glm::vec3(5.0f, -0.6f, -18.2f),
        glm::vec3(14.8f, -0.3f, -19.5f),
        glm::vec3(16.9f, -0.8f, -18.2f),
        glm::vec3(17.0f, -0.8f, -15.4f),
        glm::vec3(17.0f, -0.3f, -14.6f),
        glm::vec3(16.8f, 0.5f, -12.2f),
        glm::vec3(16.3f, 0.5f, -10.2f),
        glm::vec3(14.1f, 0.0f, -9.6f)

    };

    const std::vector<glm::vec3> AlternativeTrack = {
        glm::vec3(15.0f, 0.0f, -10.0f),
        glm::vec3(10.0f, 5.0f, -5.0f),
        glm::vec3(5.0f, -5.0f, 5.0f),
        glm::vec3(0.0f, 0.0f, 10.0f)
    };
}