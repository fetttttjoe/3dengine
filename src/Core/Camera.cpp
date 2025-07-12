#include "Core/Camera.h"

#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Core/Application.h"
#include "Core/SettingsManager.h"

Camera::Camera(GLFWwindow* window, glm::vec3 position)
    : m_Window(window),
      m_Position(position),
      m_WorldUp(0.0f, 1.0f, 0.0f),
      m_Yaw(-90.0f),
      m_Pitch(-10.0f),
      m_Zoom(45.0f),
      m_MovementSpeed(5.0f),
      m_AspectRatio(16.0f / 9.0f) {
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

void Camera::HandleInput(float deltaTime,
                         std::function<void()> onUpdateCallback) {
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

    if (xoffset != 0.0f || yoffset != 0.0f) {
      mouseMoved = processMouseMovement(xoffset, yoffset);
    }
  } else {
    m_IsOrbiting = false;
  }

  if ((keyboardMoved || mouseMoved) && onUpdateCallback) {
    onUpdateCallback();
  }
}

void Camera::ResetToDefault() {
  m_Position = glm::vec3(0.0f, 2.0f, 8.0f);
  m_Yaw = -90.0f;
  m_Pitch = -10.0f;
  m_Zoom = 45.0f;
  updateMatrices();
  Application::Get().RequestSceneRender();
}

void Camera::ProcessMouseScroll(float yoffset) {
  m_Position += m_Front * yoffset * 0.5f;
  updateMatrices();
  Application::Get().RequestSceneRender();
}

void Camera::SetAspectRatio(float aspectRatio) {
  if (m_AspectRatio != aspectRatio) {
    m_AspectRatio = aspectRatio;
    updateMatrices();
    Application::Get().RequestSceneRender();
  }
}

glm::vec2 Camera::WorldToScreen(const glm::vec3& worldPos,
                                const glm::mat4& viewProj, int windowWidth,
                                int windowHeight) {
  glm::vec4 clipPos = viewProj * glm::vec4(worldPos, 1.0f);
  glm::vec3 ndcPos = glm::vec3(clipPos) / clipPos.w;
  float screenX = (ndcPos.x + 1.0f) / 2.0f * windowWidth;
  float screenY = (1.0f - ndcPos.y) / 2.0f * windowHeight;
  return glm::vec2(screenX, screenY);
}

glm::vec3 Camera::ScreenToWorldPoint(const glm::vec2& screenPos, float ndcZ,
                                     const glm::mat4& invViewProj,
                                     int windowWidth, int windowHeight) {
  float ndcX = (screenPos.x / windowWidth) * 2.0f - 1.0f;
  float ndcY = 1.0f - (screenPos.y / windowHeight) * 2.0f;
  glm::vec4 worldPos = invViewProj * glm::vec4(ndcX, ndcY, ndcZ, 1.0f);
  if (worldPos.w == 0.0f) return glm::vec3(0.0f);
  return glm::vec3(worldPos) / worldPos.w;
}

glm::vec3 Camera::ScreenToWorldRay(const glm::vec2& screenPos, int windowWidth,
                                   int windowHeight) const {
  float ndcX = (screenPos.x / windowWidth) * 2.0f - 1.0f;
  float ndcY = 1.0f - (screenPos.y / windowHeight) * 2.0f;
  glm::vec4 rayClip(ndcX, ndcY, -1.0, 1.0);
  glm::vec4 rayEye = glm::inverse(m_ProjectionMatrix) * rayClip;
  rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);
  glm::vec3 rayWorld = glm::vec3(glm::inverse(m_ViewMatrix) * rayEye);
  return glm::normalize(rayWorld);
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
  m_ProjectionMatrix =
      glm::perspective(glm::radians(m_Zoom), m_AspectRatio, 0.1f, 100.0f);
}

bool Camera::processKeyboard(float deltaTime) {
  float velocity = SettingsManager::Get().cameraSpeed * deltaTime;
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
    Application::Get().RequestSceneRender();
  }
  return moved;
}

bool Camera::processMouseMovement(float xoffset, float yoffset) {
  float sensitivity = 0.1f;
  xoffset *= sensitivity;
  yoffset *= sensitivity;
  m_Yaw += xoffset;
  m_Pitch += yoffset;
  if (m_Pitch > 89.0f) m_Pitch = 89.0f;
  if (m_Pitch < -89.0f) m_Pitch = -89.0f;
  updateMatrices();
  Application::Get().RequestSceneRender();
  return true;
}