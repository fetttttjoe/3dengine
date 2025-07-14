// Model/src/Sculpting/Tools/BrushSettings.h
#pragma once

#include "Core/UI/Curve.h"
#include "Sculpting/ISculptTool.h"  // Includes SculptMode::Mode enum

// Encapsulates all settings for a sculpting brush
struct BrushSettings {
  float radius = 0.5f;
  float strength = 0.5f;
  // SculptMode::Mode now only contains brush-like tools
  SculptMode::Mode mode = SculptMode::Pull;
  Curve falloff;  // The falloff profile of the brush

  BrushSettings() {
    // Default to a smooth, linear falloff
    falloff.AddPoint({0.0f, 1.0f});
    falloff.AddPoint({1.0f, 0.0f});
  }
};