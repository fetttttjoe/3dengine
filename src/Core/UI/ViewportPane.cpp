#include "Core/UI/ViewportPane.h"

#include "Core/UI/UIElements.h"

ViewportPane::ViewportPane() {}

void ViewportPane::Draw(uint32_t textureId) {
  m_Size =
      UIElements::ViewportImage(textureId, m_Bounds, m_IsFocused, m_IsHovered);
}