#include "Core/Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <iostream>

// Constructor and other methods from your file remain the same...
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

glm::vec3 Camera::WorldToScreenDirection(const glm::vec3 &worldDir) const
{
    glm::mat4 vp = m_ProjectionMatrix * m_ViewMatrix;

    // Use two points to define the direction vector in world space
    glm::vec4 p1_world = glm::vec4(m_Position, 1.0f);
    glm::vec4 p2_world = glm::vec4(m_Position + worldDir, 1.0f);

    // Transform to clip space
    glm::vec4 p1_clip = vp * p1_world;
    glm::vec4 p2_clip = vp * p2_world;

    // Perspective divide to get Normalized Device Coordinates
    glm::vec3 p1_ndc = glm::vec3(p1_clip) / p1_clip.w;
    glm::vec3 p2_ndc = glm::vec3(p2_clip) / p2_clip.w;

    // Get the direction vector in NDC space and flip Y
    glm::vec3 screenDir = p2_ndc - p1_ndc;
    screenDir.y = -screenDir.y;

    return screenDir;
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
