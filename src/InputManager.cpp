#include "InputManager.h"

/**
 * @brief constructor
 * 
 * has references to the GLFW window, the current camera and the application state
 * 
 * @param window pointer to GLFW window.
 * @param camera reference to current cam
 * @param appState reference to AppState struct
 */
InputManager::InputManager(GLFWwindow* window, Camera& camera, AppState& appState)
    : m_window(window), m_camera(camera), m_state(appState) {}

/**
 * @brief verwerkt de continue toetsenbord-invoer
 * 
 * movement (WASD)
 * quit (Q),
 * release cursor (ESC)
 * scenetoggles (R, T, C, L, B, O, G)
 * post-processing filters (1, 2, 3)
 * 
 * @param deltaTime time elapsed since last frame
 * @param scene reference to the active 3D scene
 * @param postProcessor reference to the PostProcessor
 * @param bloom reference to the Bloom post-processor
 * @param chromaKey reference to the ChromaKey overlay-processor
 */
void InputManager::update(float deltaTime, Scene& scene, PostProcessor& postProcessor, Bloom& bloom, ChromaKey& chromaKey) {
    // Continue beweging via WASD als de muis vastgelegd is
    if (m_state.isMouseCaptured) {
        m_camera.ProcessKeyboard(m_window, deltaTime);
    }

    // Afsluiten (Q)
    if (glfwGetKey(m_window, GLFW_KEY_Q) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_window, true);
    }

    // Muis loslaten (ESC)
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        m_state.isMouseCaptured = false;
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    // Single key toggles
    if (isKeyPressedOnce(GLFW_KEY_R)) scene.toggleLamp();
    if (isKeyPressedOnce(GLFW_KEY_T)) scene.toggleTrack();
    if (isKeyPressedOnce(GLFW_KEY_C)) {
        m_state.beeCamera = !m_state.beeCamera;
        scene.showBee = !m_state.beeCamera;
    }
    if (isKeyPressedOnce(GLFW_KEY_L)) scene.logCameraCoordinates(m_camera.Position);
    if (isKeyPressedOnce(GLFW_KEY_B)) bloom.enabled = !bloom.enabled;
    if (isKeyPressedOnce(GLFW_KEY_O)) chromaKey.showOverlay = !chromaKey.showOverlay;
    if (isKeyPressedOnce(GLFW_KEY_G)) chromaKey.useChromaKey = !chromaKey.useChromaKey;

    // Post-processing effecten
    if (glfwGetKey(m_window, GLFW_KEY_1) == GLFW_PRESS) postProcessor.setEffect(PostEffect::NONE);
    if (glfwGetKey(m_window, GLFW_KEY_2) == GLFW_PRESS) postProcessor.setEffect(PostEffect::GAUSSIAN_BLUR);
    if (glfwGetKey(m_window, GLFW_KEY_3) == GLFW_PRESS) postProcessor.setEffect(PostEffect::EDGE_DETECT);
}

/**
 * @brief verwerkt linkermuisklik
 * 
 * if cursor not already captured, it captures the cursor and enables mouse-look
 * if cursor already captured, it performs a raycast via `scene.checkMouseClick`.
 * 
 * @param scene reference to scene
 * @param view currernt view matrix.
 * @param projection current projection matrix.
 */
void InputManager::handleMouseClick(Scene& scene, const glm::mat4& view, const glm::mat4& projection) {
    static bool leftMousePressed = false;
    
    if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (!leftMousePressed) {
            leftMousePressed = true;
            if (!m_state.isMouseCaptured) {
                m_state.isMouseCaptured = true;
                glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                m_firstMouse = true;
            } else {
                scene.checkMouseClick(view, projection, m_state.screenWidth, m_state.screenHeight);
            }
        }
    } else {
        leftMousePressed = false;
    }
}

/**
 * @brief callback methode voor muismovement
 * 
 * berekent de offset ten opzichte van het vorige frame en stuurt deze door naar de camera
 * 
 * @param xpos current X-positie van cursor
 * @param ypos current Y-positie van cursor
 */
void InputManager::onMouseMove(double xpos, double ypos) {
    if (!m_state.isMouseCaptured) return;

    if (m_firstMouse) {
        m_lastX = static_cast<float>(xpos);
        m_lastY = static_cast<float>(ypos);
        m_firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - m_lastX;
    float yoffset = m_lastY - static_cast<float>(ypos);
    m_lastX = static_cast<float>(xpos);
    m_lastY = static_cast<float>(ypos);

    m_camera.ProcessMouseMovement(xoffset, yoffset);
}

/**
 * @brief callback methode voor scrollwiel
 * 
 * @param yoffset verticale scroll-offset
 */
void InputManager::onMouseScroll(double yoffset) {
    if (!m_state.isMouseCaptured) return;
    m_camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

/**
 * @brief helper-functie voor single key press detection
 * 
 * voorkomt dat een actie meerdere keren per frame wordt geswitched
 * 
 * @param key GLFW key-code
 * @return true if the key was just pressed or not held down
 * @return false if the key is not pressed or is being held down
 */
bool InputManager::isKeyPressedOnce(int key) {
    static bool keyStates[GLFW_KEY_LAST] = { false };
    bool isPressed = glfwGetKey(m_window, key) == GLFW_PRESS;
    
    if (isPressed && !keyStates[key]) {
        keyStates[key] = true;
        return true;
    } else if (!isPressed) {
        keyStates[key] = false;
    }
    return false;
}