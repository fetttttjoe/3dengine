#pragma once
#include <glm/glm.hpp>
#include <functional> // NEW: Include for std::function (the callback)

struct GLFWwindow; // Forward declare GLFWwindow

class Camera {
public:
    Camera(GLFWwindow* window, glm::vec3 position = glm::vec3(0.0f, 2.0f, 8.0f));

    // REFACTORED: Now accepts a callback to notify when the camera has updated.
    void HandleInput(float deltaTime, std::function<void()> onUpdateCallback);
    
    void ProcessMouseScroll(float yoffset);
    void SetAspectRatio(float aspectRatio);

    const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
    const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }

private:
    void updateMatrices();
    
    // REFACTORED: These now return a boolean to indicate if they caused a change.
    bool processKeyboard(float deltaTime);
    bool processMouseMovement(float xoffset, float yoffset);

    GLFWwindow* m_Window;

    // ... other member variables remain the same ...
    glm::vec3 m_Position;
    glm::vec3 m_Front;
    glm::vec3 m_Up;
    glm::vec3 m_Right;
    glm::vec3 m_WorldUp;
    float m_Yaw;
    float m_Pitch;
    float m_Zoom;
    float m_MovementSpeed;
    float m_LastX;
    float m_LastY;
    bool m_FirstMouse = true;
    bool m_IsOrbiting = false;
    glm::mat4 m_ViewMatrix;
    glm::mat4 m_ProjectionMatrix;
    float m_AspectRatio;
};