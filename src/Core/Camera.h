// =======================================================================
// File: src/Core/Camera.h
// =======================================================================
#pragma once
#include <glm/glm.hpp>

struct GLFWwindow; // Forward declare GLFWwindow

class Camera {
public:
    // Constructor initializes camera with a window handle and optional position
    Camera(GLFWwindow* window, glm::vec3 position = glm::vec3(0.0f, 2.0f, 8.0f));

    // Handles user input for camera movement and rotation
    void HandleInput(float deltaTime);
    // Processes mouse scroll input for zooming
    void ProcessMouseScroll(float yoffset);

    // NEW: Method to set the camera's aspect ratio, typically called on window resize
    void SetAspectRatio(float aspectRatio);

    // Returns the current view matrix
    const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
    // Returns the current projection matrix
    const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }

private:
    // Updates the internal view and projection matrices based on current camera state
    void updateMatrices(); // Renamed from updateViewMatrix to reflect projection update
    // Processes keyboard input for movement
    void processKeyboard(float deltaTime);
    // Processes mouse movement for camera rotation (orbiting)
    void processMouseMovement(float xoffset, float yoffset);

    GLFWwindow* m_Window; // Pointer to the GLFW window

    // Camera vectors
    glm::vec3 m_Position;  // Camera's world position
    glm::vec3 m_Front;     // Direction camera is looking
    glm::vec3 m_Up;        // Up direction relative to camera
    glm::vec3 m_Right;     // Right direction relative to camera
    glm::vec3 m_WorldUp;   // Global up direction (usually (0,1,0))

    // Euler angles for camera orientation
    float m_Yaw;           // Yaw angle (left/right rotation)
    float m_Pitch;         // Pitch angle (up/down rotation)

    float m_Zoom;          // Field of View (FoV) for projection
    float m_MovementSpeed; // Speed of camera movement

    // Mouse state for movement/orbiting
    float m_LastX;
    float m_LastY;
    bool m_FirstMouse = true; // Flag for initial mouse position
    bool m_IsOrbiting = false; // Flag to check if right mouse button is pressed for orbiting

    // Camera matrices
    glm::mat4 m_ViewMatrix;
    glm::mat4 m_ProjectionMatrix;

    // NEW: Store aspect ratio to recreate projection matrix on demand
    float m_AspectRatio;
};