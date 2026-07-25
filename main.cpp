#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Shader.h>
#include <Camera.h>
#include <Scene.h>
#include <PostProcess.h>
#include <Bloom.h>
#include <ChromaKey.h>
#include "InputManager.h"

// globale vars
Camera camera;
AppState appState;
std::unique_ptr<InputManager> inputManager;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Callbacks naar InputManager
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (inputManager) inputManager->onMouseMove(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (inputManager) inputManager->onMouseScroll(yoffset);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    appState.screenWidth = width;
    appState.screenHeight = height;
    appState.windowResized = true;
}

GLFWwindow* initWindow(int width, int height) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, "LearnOpenGL", NULL, NULL);
    if (!window) { 
        glfwTerminate(); 
        return nullptr; 
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) 
        return nullptr;
    
    return window;
}

int main() {
    GLFWwindow* window = initWindow(appState.screenWidth, appState.screenHeight);
    if (!window) return -1;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0, 0, appState.screenWidth, appState.screenHeight);

    // init managers en scene
    inputManager = std::make_unique<InputManager>(window, camera, appState);
    
    Shader lightingShader("shaders/lighting.vert", "shaders/lighting.frag");
    Shader lampShader("shaders/light.vert", "shaders/light.frag");
    Scene scene;

    auto postProcessor = std::make_unique<PostProcessor>(appState.screenWidth, appState.screenHeight);
    auto bloom         = std::make_unique<Bloom>(appState.screenWidth, appState.screenHeight);
    // inladen chroma key
    ChromaKey chromaKey("textures/spongebob.jpg");

    // --- RENDER LOOP ---
    while (!glfwWindowShouldClose(window)) {
        // 1. Tijd updates
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Resizing afhandelen
        if (appState.windowResized && appState.screenWidth > 0 && appState.screenHeight > 0) {
            bloom = std::make_unique<Bloom>(appState.screenWidth, appState.screenHeight);
            postProcessor = std::make_unique<PostProcessor>(appState.screenWidth, appState.screenHeight);
            appState.windowResized = false;
        }

        // 2. Input verwerken
        inputManager->update(deltaTime, scene, *postProcessor, *bloom, chromaKey);

        // 3. Clear Buffers 
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 4. Matrixen bereken
        glm::mat4 view = appState.beeCamera ? 
            glm::lookAt(scene.getBeePosition(), scene.getBeePosition() + scene.getBeeDirection(), glm::vec3(0.0f, 1.0f, 0.0f)) : 
            camera.GetViewMatrix();

        glm::mat4 projection = glm::perspective(
            glm::radians(camera.Fov), 
            (float)appState.screenWidth / (float)appState.screenHeight, 
            0.1f, 300.0f
        );

        // 4. Muisklik afhandelen
        inputManager->handleMouseClick(scene, view, projection);

        // 5. Render Pipeline
        bloom->bindScene();
        scene.Draw(lightingShader, lampShader, view, projection, camera.Position);
        chromaKey.DrawPlane();

        bloom->process();
        bloom->render();
        postProcessor->Draw(bloom->getResultTexture());

        // 6. Swap Buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}