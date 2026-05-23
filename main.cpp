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

Camera camera;
bool firstMouse = true;
float lastX = 400.0f, lastY = 300.0f;
float deltaTime = 0.0f, lastFrame = 0.0f;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1920, 1080, "LearnOpenGL", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    // aanzetten van depth buffer
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0, 0, 1920, 1080);

    // inladen van shaders
    Shader lightingShader("shaders/lighting.vert", "shaders/lighting.frag");
    Shader lampShader("shaders/light.vert", "shaders/light.frag");
    Scene scene;
    // inladen postprocessor:
    PostProcessor postProcessor(1920, 1080);
    Bloom bloom(1920, 1080);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // sluiten van window door escape key
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        camera.ProcessKeyboard(window, deltaTime);

        static bool rKeyPressed = false;

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            // Als hij is ingedrukt, en hij was de vorige frame nog NIET ingedrukt
            if (!rKeyPressed) {
                // Draai de schakelaar om (van true naar false of andersom)
                scene.redstoneLampsOn = !scene.redstoneLampsOn;
                rKeyPressed = true; // Zet op true zodat hij niet blijft flipperen
            }
        } else {
            // Zodra je de knop loslaat, resetten we de beveiliging
            rKeyPressed = false;
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        // depth buffer instellen en aanzetten
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // matrixen verkrijgen
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), 1920.0f / 1080.0f, 0.1f, 300.0f);

    // tekenen van objecten en postprocesser en bloom renderen
        static bool leftMousePressed = false;
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            if (!leftMousePressed) {
                leftMousePressed = true;
                scene.checkMouseClick(view, projection, camera.Position);
            }
        } else {
            leftMousePressed = false;
        }

        bloom.bindScene();
        scene.Draw(lightingShader, lampShader, view, projection, camera.Position);
        bloom.process();
        bloom.render();
        postProcessor.DrawFromTexture(bloom.getResultTexture());


        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
            postProcessor.setEffect(PostEffect::NONE);
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
            postProcessor.setEffect(PostEffect::GAUSSIAN_BLUR);
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
            postProcessor.setEffect(PostEffect::EDGE_DETECT);
        static bool bWasPressed = false;
        bool bPressed = glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS;
        if (bPressed && !bWasPressed) {
            bloom.enabled = !bloom.enabled;
            std::cout << "Bloom: " << (bloom.enabled ? "AAN" : "UIT") << std::endl;
        }
        bWasPressed = bPressed;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup en afsluiten van scene
    scene.Delete();
    postProcessor.Delete();
    bloom.Delete();
    glfwTerminate();
    return 0;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX, yoffset = lastY - ypos;
    lastX = xpos; lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
