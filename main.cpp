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

// struct for input related state
struct AppState {
    bool beeCamera = false;
    bool isMouseCaptured = false;
    bool windowResized = false;
    int screenWidth = 1920;
    int screenHeight = 1080;
};

// globale vars
Camera camera;
AppState appState;
bool firstMouse = true;
float lastX = 960.0f, lastY = 540.0f;
float deltaTime = 0.0f, lastFrame = 0.0f;

// Callback prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, Scene& scene, PostProcessor& postProcessor, Bloom& bloom, ChromaKey& chromaKey, AppState& state);

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

    // inladen van shaders, scene & effects
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
        if (appState.isMouseCaptured) {
            camera.ProcessKeyboard(window, deltaTime);
        }        
        processInput(window, scene, *postProcessor, *bloom, chromaKey, appState);

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

        // 5. Muisklik check
        static bool leftMousePressed = false;
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            if (!leftMousePressed) {
                leftMousePressed = true;
                if (!appState.isMouseCaptured) {
                    appState.isMouseCaptured = true;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    firstMouse = true;
                } else {
                    scene.checkMouseClick(view, projection, appState.screenWidth, appState.screenHeight);
                }
            }
        } else {
            leftMousePressed = false;
        }

        // 6. Render Pipeline (Scene -> Bloom -> PostProcess)
        bloom->bindScene();
        scene.Draw(lightingShader, lampShader, view, projection, camera.Position);
        chromaKey.DrawPlane();

        bloom->process();
        bloom->render();
        postProcessor->DrawFromTexture(bloom->getResultTexture());

        // 7. swap Buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Geen 'delete' nodig! unique_ptr ruimt alles automatisch netjes op
    glfwTerminate();
    return 0;
}

// --- CALLBACKS ---

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!appState.isMouseCaptured) return;

    if (firstMouse) { 
        lastX = static_cast<float>(xpos); 
        lastY = static_cast<float>(ypos); 
        firstMouse = false; 
    }
    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos);
    lastX = static_cast<float>(xpos); 
    lastY = static_cast<float>(ypos);
    
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (!appState.isMouseCaptured) return;
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    appState.screenWidth = width;
    appState.screenHeight = height;
    appState.windowResized = true;
}

// --- INPUT HANDLER ---

void processInput(GLFWwindow* window, Scene& scene, PostProcessor& postProcessor, Bloom& bloom, ChromaKey& chromaKey, AppState& state) {
    // afsluiten (Q)
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
        
    // muis loslaten (ESC)
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        state.isMouseCaptured = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    // single key-presses helper
    auto isKeyPressedOnce = [window](int key) {
        static bool keyStates[GLFW_KEY_LAST] = { false };
        bool isPressed = glfwGetKey(window, key) == GLFW_PRESS;
        if (isPressed && !keyStates[key]) {
            keyStates[key] = true;
            return true;
        } else if (!isPressed) {
            keyStates[key] = false;
        }
        return false;
    };

    // Toggles
    if (isKeyPressedOnce(GLFW_KEY_R)) scene.toggleLamp();
    if (isKeyPressedOnce(GLFW_KEY_T)) scene.toggleTrack();
    if (isKeyPressedOnce(GLFW_KEY_C)) {
        state.beeCamera = !state.beeCamera;
        scene.showBee = !state.beeCamera;
    }
    if (isKeyPressedOnce(GLFW_KEY_L)) scene.logCameraCoordinates(camera.Position);
    if (isKeyPressedOnce(GLFW_KEY_B)) bloom.enabled = !bloom.enabled;
    if (isKeyPressedOnce(GLFW_KEY_O)) chromaKey.showOverlay = !chromaKey.showOverlay;
    if (isKeyPressedOnce(GLFW_KEY_G)) chromaKey.useChromaKey = !chromaKey.useChromaKey;

    // Post-processing effecten
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) postProcessor.setEffect(PostEffect::NONE);
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) postProcessor.setEffect(PostEffect::GAUSSIAN_BLUR);
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) postProcessor.setEffect(PostEffect::EDGE_DETECT);
}