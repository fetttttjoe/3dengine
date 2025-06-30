
// =======================================================================
// File: src/Camera.cpp (NEW FILE)
// =======================================================================
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h> 

Camera::Camera(GLFWwindow* window, glm::vec3 position) :
    m_Window(window),
    m_Position(position),
    m_WorldUp(0.0f, 1.0f, 0.0f),
    m_Yaw(-90.0f),
    m_Pitch(0.0f),
    m_Zoom(45.0f)
{
    double xpos, ypos;
    glfwGetCursorPos(m_Window, &xpos, &ypos);
    m_LastX = (float)xpos;
    m_LastY = (float)ypos;
    updateViewMatrix();
}

void Camera::HandleInput() {
    // --- Orbiting ---
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
        processMouseMovement(xoffset, yoffset);
    } else {
        m_IsOrbiting = false;
    }

    // --- Zooming ---
    // This requires setting a scroll callback in Application.cpp
    // For simplicity, we'll poll it here. A callback is better.
    // Note: This part is a placeholder for a proper scroll callback.
}

void Camera::updateViewMatrix() {
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);
    
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));

    // Simple zoom implementation by moving camera along its front vector
    m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);

    // Update projection matrix
    int width, height;
    glfwGetWindowSize(m_Window, &width, &height);
    if (height > 0) {
        float aspectRatio = (float)width / (float)height;
        m_ProjectionMatrix = glm::perspective(glm::radians(m_Zoom), aspectRatio, 0.1f, 100.0f);
    }
}

void Camera::processMouseMovement(float xoffset, float yoffset) {
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    m_Yaw += xoffset;
    m_Pitch += yoffset;

    if (m_Pitch > 89.0f) m_Pitch = 89.0f;
    if (m_Pitch < -89.0f) m_Pitch = -89.0f;

    updateViewMatrix();
}

void Camera::ProcessMouseScroll(float yoffset) {
    m_Zoom -= (float)yoffset;
    if (m_Zoom < 1.0f) m_Zoom = 1.0f;
    if (m_Zoom > 45.0f) m_Zoom = 45.0f;
    updateViewMatrix();
}

