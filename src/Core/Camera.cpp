#include "Core/Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <iostream>

// Constructor remains the same
Camera::Camera(GLFWwindow* window, glm::vec3 position) :
    m_Window(window),
    m_Position(position),
    m_WorldUp(0.0f, 1.0f, 0.0f),
    m_Yaw(-90.0f),
    m_Pitch(-10.0f),
    m_Zoom(45.0f),
    m_MovementSpeed(5.0f),
    m_AspectRatio(16.0f / 9.0f)
{
    double xpos, ypos;
    glfwGetCursorPos(m_Window, &xpos, &ypos);
    m_LastX = (float)xpos;
    m_LastY = (float)ypos;
    int width, height;
    glfwGetWindowSize(m_Window, &width, &height);
    if (height > 0) {
        m_AspectRatio = (float)width / (float)height;
    }
    updateMatrices();
}

// REFACTORED: Main input handler now calls the callback if a change occurs.
void Camera::HandleInput(float deltaTime, std::function<void()> onUpdateCallback) {
    bool keyboardMoved = processKeyboard(deltaTime);
    bool mouseMoved = false;

    if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        if (!m_IsOrbiting) {
            m_IsOrbiting = true;
            m_FirstMouse = true;
        }
        double xpos, ypos;
        glfwGetCursorPos(m_Window, &xpos, &ypos);
        if (m_FirstMouse) {
            m_LastX = (float)xpos;
            m_LastY = (float)ypos;
            m_FirstMouse = false;
        }
        float xoffset = (float)xpos - m_LastX;
        float yoffset = m_LastY - (float)ypos;
        m_LastX = (float)xpos;
        m_LastY = (float)ypos;

        // Only consider it "moved" if there was a non-zero offset
        if (xoffset != 0.0f || yoffset != 0.0f) {
            mouseMoved = processMouseMovement(xoffset, yoffset);
        }
    } else {
        m_IsOrbiting = false;
    }
    
    // If either input method changed the camera, trigger the callback.
    if ((keyboardMoved || mouseMoved) && onUpdateCallback) {
        onUpdateCallback();
    }
}

// NOTE: No changes needed here. The Application's scroll_callback handles
// requesting the redraw after calling this function.
void Camera::ProcessMouseScroll(float yoffset) {
    m_Position += m_Front * yoffset * 0.5f;
    updateMatrices();
}

void Camera::SetAspectRatio(float aspectRatio) {
    if (m_AspectRatio != aspectRatio) {
        m_AspectRatio = aspectRatio;
        updateMatrices();
    }
}

void Camera::updateMatrices() {
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
    m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
    m_ProjectionMatrix = glm::perspective(glm::radians(m_Zoom), m_AspectRatio, 0.1f, 100.0f);
}

// REFACTORED: Now returns true if a key was pressed, false otherwise.
bool Camera::processKeyboard(float deltaTime) {
    float velocity = m_MovementSpeed * deltaTime;
    bool moved = false;

    if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS) {
        m_Position += m_Front * velocity;
        moved = true;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS) {
        m_Position -= m_Front * velocity;
        moved = true;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS) {
        m_Position -= m_Right * velocity;
        moved = true;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS) {
        m_Position += m_Right * velocity;
        moved = true;
    }
    
    if (moved) {
        updateMatrices();
    }
    return moved;
}

// REFACTORED: Now returns true, as any call to it means the mouse moved.
bool Camera::processMouseMovement(float xoffset, float yoffset) {
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    m_Yaw += xoffset;
    m_Pitch += yoffset;

    if (m_Pitch > 89.0f) m_Pitch = 89.0f;
    if (m_Pitch < -89.0f) m_Pitch = -89.0f;

    updateMatrices();
    return true; // If this function is called, it means the mouse moved.
}