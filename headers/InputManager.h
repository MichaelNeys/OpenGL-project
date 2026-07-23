#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <Camera.h>
#include <Scene.h>
#include <PostProcess.h>
#include <Bloom.h>
#include <ChromaKey.h>

struct AppState {
    bool beeCamera = false;
    bool isMouseCaptured = false;
    bool windowResized = false;
    int screenWidth = 1920;
    int screenHeight = 1080;
};

class InputManager {
public:
    InputManager(GLFWwindow* window, Camera& camera, AppState& appState);

    void update(float deltaTime, Scene& scene, PostProcessor& postProcessor, Bloom& bloom, ChromaKey& chromaKey);

    void handleMouseClick(Scene& scene, const glm::mat4& view, const glm::mat4& projection);

    // Callbacks
    void onMouseMove(double xpos, double ypos);
    void onMouseScroll(double yoffset);

private:
    GLFWwindow* m_window;
    Camera& m_camera;
    AppState& m_state;

    bool m_firstMouse = true;
    float m_lastX = 960.0f;
    float m_lastY = 540.0f;

    bool isKeyPressedOnce(int key);
};