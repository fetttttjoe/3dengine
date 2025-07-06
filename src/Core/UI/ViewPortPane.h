#pragma once

#include <array>
#include <cstdint>
#include <glm/glm.hpp>

#include "imgui.h"

class ViewportPane {
 public:
  ViewportPane();
  void Draw(uint32_t textureId);

  glm::vec2 GetSize() const { return {m_Size.x, m_Size.y}; }
  const std::array<ImVec2, 2>& GetBounds() const { return m_Bounds; }
  bool IsFocused() const { return m_IsFocused; }
  bool IsHovered() const { return m_IsHovered; }

 private:
  ImVec2 m_Size = {0, 0};
  std::array<ImVec2, 2> m_Bounds = {};
  bool m_IsFocused = false;
  bool m_IsHovered = false;
};