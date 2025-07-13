#pragma once
#include <functional>
#include <glm/glm.hpp>

#include "Core/Log.h"

struct GLFWwindow;

class Camera {
 public:
  Camera(GLFWwindow* window, glm::vec3 position = glm::vec3(0.0f, 2.0f, 8.0f));

  void ResetToDefault();
  void HandleInput(float deltaTime, std::function<void()> onUpdateCallback);

  void ProcessMouseScroll(float yoffset);
  void SetAspectRatio(float aspectRatio);
  
  void SetPosition(const glm::vec3& position) {
      m_Position = position;
      updateMatrices();
  }
  // NEW: Add public setters for Yaw and Pitch for test control
  void SetYaw(float yaw) {
      m_Yaw = yaw;
      updateMatrices(); // Recalculate matrices after yaw/pitch change
  }
  void SetPitch(float pitch) {
      m_Pitch = pitch;
      updateMatrices(); // Recalculate matrices after yaw/pitch change
  }


  const glm::mat4& GetViewMatrix() const {
    Log::Debug("Camera::GetViewMatrix() called.");
    return m_ViewMatrix;
  }
  const glm::mat4& GetProjectionMatrix() const {
    Log::Debug("Camera::GetProjectionMatrix() called.");
    return m_ProjectionMatrix;
  }

  glm::vec3 GetPosition() const { return m_Position; }
  glm::vec3 GetFront() const { return m_Front; } // Getter for m_Front is already there


  static glm::vec2 WorldToScreen(const glm::vec3& worldPos,
                                 const glm::mat4& viewProj, int windowWidth,
                                 int windowHeight);
  static glm::vec3 ScreenToWorldPoint(const glm::vec2& screenPos, float ndcZ,
                                      const glm::mat4& invViewProj,
                                      int windowWidth, int windowHeight);
  glm::vec3 ScreenToWorldRay(const glm::vec2& screenPos, int windowWidth,
                             int windowHeight) const;

 private:
  void updateMatrices();

  bool processKeyboard(float deltaTime);
  bool processMouseMovement(float xoffset, float yoffset);

  GLFWwindow* m_Window;

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