#pragma once

#include <array>
#include <cstdint>
#include <glm/glm.hpp>

#include "Core/UI/IView.h"
#include "imgui.h"

class Application;
class OpenGLRenderer;

class ViewportPane : public IView {
 public:
  explicit ViewportPane(Application* app);

  void Draw() override;
  const char* GetName() const override { return "ViewportPane"; }

  glm::vec2 GetSize() const { return {m_Size.x, m_Size.y}; }
  const std::array<ImVec2, 2>& GetBounds() const { return m_Bounds; }
  bool IsFocused() const { return m_IsFocused; }
  bool IsHovered() const { return m_IsHovered; }

 private:
  Application* m_App;
  OpenGLRenderer* m_Renderer;  // Cache renderer pointer

  ImVec2 m_Size = {0, 0};
  std::array<ImVec2, 2> m_Bounds = {};
  bool m_IsFocused = false;
  bool m_IsHovered = false;
};
