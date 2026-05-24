#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <shader.h>
#include <camera.h>
#include <scene.h>
#include <postprocess.h>
#include <bloom.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window, Scene& scene, PostProcessor& postProcessor, Bloom& bloom, bool& beeCamera, bool& isMouseCaptured);

Camera camera;
bool firstMouse = true;
bool isMouseCaptured = false; 
float lastX = 400.0f, lastY = 300.0f;
float deltaTime = 0.0f, lastFrame = 0.0f;

int screenWidth = 1920;
int screenHeight = 1080;
bool windowResized = false;

GLFWwindow* initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "LearnOpenGL", NULL, NULL);
    if (!window) { glfwTerminate(); return nullptr; }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return nullptr;
    
    return window;
}

int main() {
    GLFWwindow* window = initWindow();
    if (!window) return -1;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0, 0, screenWidth, screenHeight);

    // inladen van shaders
    Shader lightingShader("shaders/lighting.vert", "shaders/lighting.frag");
    Shader lampShader("shaders/light.vert", "shaders/light.frag");
    Scene scene;
    // inladen postprocessor:
    PostProcessor* postProcessor = new PostProcessor(screenWidth, screenHeight);
    Bloom* bloom = new Bloom(screenWidth, screenHeight);

    bool beeCamera = false;

    while (!glfwWindowShouldClose(window)) {
        // --- 1. Tijd en Frame updates ---
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // updates wanneer resized
        if (windowResized && screenWidth > 0 && screenHeight > 0) {
            bloom->Delete();
            postProcessor->Delete();
            delete bloom;
            delete postProcessor;

            bloom = new Bloom(screenWidth, screenHeight);
            postProcessor = new PostProcessor(screenWidth, screenHeight);
            
            windowResized = false;
        }

        // --- 2. Input verwerken ---
        if (isMouseCaptured) {
            camera.ProcessKeyboard(window, deltaTime);
        }        
        processInput(window, scene, *postProcessor, *bloom, beeCamera, isMouseCaptured);

        // --- 3. Scherm schoonmaken ---
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- 4. Matrixen berekenen ---
        glm::mat4 view;
        if (beeCamera) {
            glm::vec3 beePos = scene.getBeePosition();
            glm::vec3 beeDir = scene.getBeeDirection();
            view = glm::lookAt(beePos, beePos + beeDir, glm::vec3(0.0f, 1.0f, 0.0f));
        } else {
            view = camera.GetViewMatrix();
        }
        glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)screenWidth / (float)screenHeight, 0.1f, 300.0f);

        // Muisklik check
        static bool leftMousePressed = false;
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            if (!leftMousePressed) {
                leftMousePressed = true;
                
                if (!isMouseCaptured) {
                    isMouseCaptured = true;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    firstMouse = true;
                } else {
                    scene.checkMouseClick(view, projection, screenWidth, screenHeight);
                }
            }
        } else {
            leftMousePressed = false;
        }

        // --- 5. Renderen (Tekenen) ---
        bloom->bindScene();
        scene.Draw(lightingShader, lampShader, view, projection, camera.Position);
        bloom->process();
        bloom->render();
        postProcessor->DrawFromTexture(bloom->getResultTexture());

        // --- 6. Buffers wisselen en events pollen ---
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup en afsluiten van scene
    scene.Delete();
    postProcessor->Delete();
    bloom->Delete();
    delete postProcessor;
    delete bloom;
    
    glfwTerminate();
    return 0;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!isMouseCaptured) return;

    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX, yoffset = lastY - ypos;
    lastX = xpos; lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (!isMouseCaptured) return;
    camera.ProcessMouseScroll(yoffset);
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    screenWidth = width;
    screenHeight = height;
    windowResized = true;
}

void processInput(GLFWwindow *window, Scene& scene, PostProcessor& postProcessor, Bloom& bloom, bool& beeCamera, bool& isMouseCaptured) {
    // afsluiten (Q)
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
        
    // muis loslaten (ESC)
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        isMouseCaptured = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    // lampen toggelen (R)
    static bool rWasPressed = false;
    bool rPressed = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
    if (rPressed && !rWasPressed) scene.redstoneLampsOn = !scene.redstoneLampsOn;
    rWasPressed = rPressed;

    // Bee camera togglen (C)
    static bool cWasPressed = false;
    bool cPressed = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;
    if (cPressed && !cWasPressed) {
        beeCamera = !beeCamera;
        scene.showBee = !beeCamera;
    }
    cWasPressed = cPressed;

    // Bloom togglen (B)
    static bool bWasPressed = false;
    bool bPressed = glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS;
    if (bPressed && !bWasPressed) bloom.enabled = !bloom.enabled;
    bWasPressed = bPressed;

    // Post-processing effecten
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) postProcessor.setEffect(PostEffect::NONE);
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) postProcessor.setEffect(PostEffect::GAUSSIAN_BLUR);
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) postProcessor.setEffect(PostEffect::EDGE_DETECT);
}