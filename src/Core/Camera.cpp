// =======================================================================
// File: src/Core/Camera.cpp
// =======================================================================
#include "Core/Camera.h"
#include <glm/gtc/matrix_transform.hpp> // For glm::lookAt, glm::perspective
#include <GLFW/glfw3.h>                 // For GLFW functions (get cursor pos, get window size)
#include <iostream>                     // For potential debug output

Camera::Camera(GLFWwindow* window, glm::vec3 position) :
    m_Window(window),
    m_Position(position),
    m_WorldUp(0.0f, 1.0f, 0.0f),
    m_Yaw(-90.0f),      // Initial yaw (looking along -Z axis by default)
    m_Pitch(-10.0f),    // Initial pitch (slightly looking down)
    m_Zoom(45.0f),      // Initial Field of View
    m_MovementSpeed(5.0f), // Initial movement speed
    m_AspectRatio(16.0f / 9.0f) // Default aspect ratio, will be updated by SetAspectRatio
{
    // Get initial mouse position
    double xpos, ypos;
    glfwGetCursorPos(m_Window, &xpos, &ypos);
    m_LastX = (float)xpos;
    m_LastY = (float)ypos;

    // Get initial window size to set correct aspect ratio for projection matrix
    int width, height;
    glfwGetWindowSize(m_Window, &width, &height);
    if (height > 0) {
        m_AspectRatio = (float)width / (float)height;
    }

    updateMatrices(); // Call the updated method to set up both view and projection
}

void Camera::HandleInput(float deltaTime) {
    processKeyboard(deltaTime);

    // Only process mouse movement for orbiting if right mouse button is pressed
    if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        // If this is the start of an orbit session, reset m_FirstMouse
        if (!m_IsOrbiting) {
            m_IsOrbiting = true;
            m_FirstMouse = true; // Reset first mouse flag when orbiting starts
        }
        double xpos, ypos;
        glfwGetCursorPos(m_Window, &xpos, &ypos);

        // On the very first mouse movement after starting orbit,
        // set last X/Y to current position to prevent a jump
        if (m_FirstMouse) {
            m_LastX = (float)xpos;
            m_LastY = (float)ypos;
            m_FirstMouse = false;
        }

        // Calculate offset from last frame's position
        float xoffset = (float)xpos - m_LastX;
        float yoffset = m_LastY - (float)ypos; // Inverted for typical Y-axis movement

        // Update last mouse position
        m_LastX = (float)xpos;
        m_LastY = (float)ypos;

        processMouseMovement(xoffset, yoffset);
    } else {
        m_IsOrbiting = false; // Reset orbiting flag when right mouse button is released
    }
}

void Camera::ProcessMouseScroll(float yoffset) {
    // Adjust camera position along its front vector for zooming
    // 'yoffset' typically positive for scrolling up (zoom in), negative for scrolling down (zoom out)
    m_Position += m_Front * yoffset * 0.5f; // Adjust 0.5f for desired zoom speed
    updateMatrices(); // Update view matrix after position change
}

// NEW: Method to set the camera's aspect ratio
void Camera::SetAspectRatio(float aspectRatio) {
    if (m_AspectRatio != aspectRatio) { // Only update if aspect ratio has actually changed
        m_AspectRatio = aspectRatio;
        updateMatrices(); // Recompute projection matrix
    }
}

// Consolidated method to update both view and projection matrices
void Camera::updateMatrices() {
    // Calculate the new Front vector from Euler angles
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);

    // Also re-calculate the Right and Up vector
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp)); // Normalize vectors, because their length gets closer to 0 the more you look up or down.
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));

    // Calculate the View Matrix (camera's inverse transform)
    m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);

    // Calculate the Projection Matrix (perspective projection)
    // The aspect ratio now comes from the stored m_AspectRatio
    m_ProjectionMatrix = glm::perspective(glm::radians(m_Zoom), m_AspectRatio, 0.1f, 100.0f);
}

void Camera::processKeyboard(float deltaTime) {
    float velocity = m_MovementSpeed * deltaTime;
    // Move forward/backward relative to camera's front direction
    if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS)
        m_Position += m_Front * velocity;
    if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS)
        m_Position -= m_Front * velocity;
    // Move left/right relative to camera's right direction (strafe)
    if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS)
        m_Position -= m_Right * velocity;
    if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS)
        m_Position += m_Right * velocity;

    updateMatrices(); // Update view matrix after position change
}

void Camera::processMouseMovement(float xoffset, float yoffset) {
    float sensitivity = 0.1f; // Mouse sensitivity for rotation
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    // Apply offsets to Yaw and Pitch
    m_Yaw += xoffset;
    m_Pitch += yoffset;

    // Constrain Pitch to avoid flipping the camera
    if (m_Pitch > 89.0f) m_Pitch = 89.0f;
    if (m_Pitch < -89.0f) m_Pitch = -89.0f;

    updateMatrices(); // Update view matrix after rotation change
}