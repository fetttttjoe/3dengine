#pragma once

#include <functional>

#include "Sculpting/ISculptTool.h"

/**
 * @class SculptPane
 * @brief The UI panel for sculpting tools and settings.
 */
class SculptPane {
 public:
  SculptPane();
  void Draw();

  // Callbacks for actions
  std::function<void(bool)> OnToggleSculptMode;

  // Brush settings
  float GetBrushRadius() const { return m_BrushRadius; }
  float GetBrushStrength() const { return m_BrushStrength; }
  SculptMode::Mode GetSculptMode() const { return m_CurrentMode; }

 private:
  float m_BrushRadius = 0.5f;
  float m_BrushStrength = 0.5f;
  SculptMode::Mode m_CurrentMode = SculptMode::Pull;
};
