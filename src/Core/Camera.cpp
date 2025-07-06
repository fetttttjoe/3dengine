#include "Core/Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <GLFW/glfw3.h>
#include <iostream>

Camera::Camera(GLFWwindow *window, glm::vec3 position) : m_Window(window),
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
    if (height > 0)
    {
        m_AspectRatio = (float)width / (float)height;
    }
    updateMatrices();
}

void Camera::HandleInput(float deltaTime, std::function<void()> onUpdateCallback)
{
    bool keyboardMoved = processKeyboard(deltaTime);
    bool mouseMoved = false;

    if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        if (!m_IsOrbiting)
        {
            m_IsOrbiting = true;
            m_FirstMouse = true;
        }
        double xpos, ypos;
        glfwGetCursorPos(m_Window, &xpos, &ypos);
        if (m_FirstMouse)
        {
            m_LastX = (float)xpos;
            m_LastY = (float)ypos;
            m_FirstMouse = false;
        }
        float xoffset = (float)xpos - m_LastX;
        float yoffset = m_LastY - (float)ypos;
        m_LastX = (float)xpos;
        m_LastY = (float)ypos;

        if (xoffset != 0.0f || yoffset != 0.0f)
        {
            mouseMoved = processMouseMovement(xoffset, yoffset);
        }
    }
    else
    {
        m_IsOrbiting = false;
    }

    if ((keyboardMoved || mouseMoved) && onUpdateCallback)
    {
        onUpdateCallback();
    }
}

void Camera::ProcessMouseScroll(float yoffset)
{
    m_Position += m_Front * yoffset * 0.5f;
    updateMatrices();
}

void Camera::SetAspectRatio(float aspectRatio)
{
    if (m_AspectRatio != aspectRatio)
    {
        m_AspectRatio = aspectRatio;
        updateMatrices();
    }
}

glm::vec2 Camera::WorldToScreen(const glm::vec3& worldPos, int windowWidth, int windowHeight) const {
    glm::mat4 vp = m_ProjectionMatrix * m_ViewMatrix;
    glm::vec4 clipPos = vp * glm::vec4(worldPos, 1.0f);

    // Perspective divide to get Normalized Device Coordinates (NDC)
    glm::vec3 ndcPos = glm::vec3(clipPos) / clipPos.w;

    // Convert NDC [-1, 1] to screen coordinates [0, width/height]
    float screenX = (ndcPos.x + 1.0f) / 2.0f * windowWidth;
    // Invert Y because screen coordinates usually have (0,0) at the top-left
    float screenY = (1.0f - ndcPos.y) / 2.0f * windowHeight;

    return glm::vec2(screenX, screenY);
}

// IMPROVEMENT: Renamed 'depth' parameter to 'ndcZ' for clarity.
glm::vec3 Camera::ScreenToWorldPoint(const glm::vec2& screenPos, float ndcZ, int windowWidth, int windowHeight) const {
    // Convert screen coordinates [0, width/height] to NDC [-1, 1]
    float ndcX = (screenPos.x / windowWidth) * 2.0f - 1.0f;
    float ndcY = 1.0f - (screenPos.y / windowHeight) * 2.0f; // Invert Y
    
    // Unproject the NDC point back to world space
    glm::mat4 invVP = glm::inverse(m_ProjectionMatrix * m_ViewMatrix);
    glm::vec4 worldPos = invVP * glm::vec4(ndcX, ndcY, ndcZ, 1.0f);

    // The result of the multiplication is in homogeneous coordinates,
    // so we must perform the perspective divide to get the final 3D point.
    if (worldPos.w == 0.0f) return glm::vec3(0.0f); // Avoid division by zero
    return glm::vec3(worldPos) / worldPos.w;
}


void Camera::updateMatrices()
{
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

bool Camera::processKeyboard(float deltaTime)
{
    float velocity = m_MovementSpeed * deltaTime;
    bool moved = false;

    if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS)
    {
        m_Position += m_Front * velocity;
        moved = true;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS)
    {
        m_Position -= m_Front * velocity;
        moved = true;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS)
    {
        m_Position -= m_Right * velocity;
        moved = true;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS)
    {
        m_Position += m_Right * velocity;
        moved = true;
    }

    if (moved)
    {
        updateMatrices();
    }
    return moved;
}

bool Camera::processMouseMovement(float xoffset, float yoffset)
{
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    m_Yaw += xoffset;
    m_Pitch += yoffset;

    if (m_Pitch > 89.0f)
        m_Pitch = 89.0f;
    if (m_Pitch < -89.0f)
        m_Pitch = -89.0f;

    updateMatrices();
    return true;
}
