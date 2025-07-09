#include "Core/UI/ViewportPane.h"

#include "Core/Application.h"
#include "Core/UI/UIElements.h"
#include "Renderer/OpenGLRenderer.h"

ViewportPane::ViewportPane(Application* app)
    : m_App(app), m_Renderer(app->GetRenderer()) {}

void ViewportPane::Draw() {
  uint32_t textureId = m_Renderer->GetSceneTextureId();
  m_Size =
      UIElements::ViewportImage(textureId, m_Bounds, m_IsFocused, m_IsHovered);
}