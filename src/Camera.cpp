#include <Camera.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

Camera::Camera(glm::vec3 position)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
      MovementSpeed(2.5f),
      MouseSensitivity(0.1f),
      Fov(45.0f),
      Yaw(-90.0f),
      Pitch(0.0f),
      WorldUp(glm::vec3(0.0f, 1.0f, 0.0f))
{
    Position = position;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() {
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(GLFWwindow* window, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    glm::vec3 flatFront = glm::normalize(glm::vec3(Front.x, 0.0f, Front.z));
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        Position += flatFront * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        Position -= flatFront * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        Position -= Right * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        Position += Right * velocity;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        Position.y += velocity;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        Position.y -= velocity;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        MovementSpeed = 10.0f;
    else
        MovementSpeed = 2.5f;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    if (Pitch > 89.0f) Pitch = 89.0f;
    if (Pitch < -89.0f) Pitch = -89.0f;

    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset) {
    Fov -= yoffset;
    if (Fov < 1.0f) Fov = 1.0f;
    if (Fov > 45.0f) Fov = 45.0f;
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}