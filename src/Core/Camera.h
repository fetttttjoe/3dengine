#pragma once
#include <functional>
#include <glm/glm.hpp>

struct GLFWwindow;

class Camera {
 public:
  Camera(GLFWwindow* window, glm::vec3 position = glm::vec3(0.0f, 2.0f, 8.0f));

  void HandleInput(float deltaTime, std::function<void()> onUpdateCallback);

  void ProcessMouseScroll(float yoffset);
  void SetAspectRatio(float aspectRatio);

  const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
  const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }

  glm::vec3 GetPosition() const { return m_Position; }

  // Helpers to convert between world and screen coordinates
  glm::vec2 WorldToScreen(const glm::vec3& worldPos, int windowWidth,
                          int windowHeight) const;

  // IMPROVEMENT: Renamed 'depth' parameter to 'ndcZ' for clarity. It expects a
  // depth value in Normalized Device Coordinates [-1, 1], not world units.
  glm::vec3 ScreenToWorldPoint(const glm::vec2& screenPos, float ndcZ,
                               int windowWidth, int windowHeight) const;

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
