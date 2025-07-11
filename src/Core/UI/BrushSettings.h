#pragma once

#include "Sculpting/ISculptTool.h"
#include "Core/UI/Curve.h"

// Encapsulates all settings for a sculpting brush
struct BrushSettings {
    float radius = 0.5f;
    float strength = 1.0f; // Increased default strength
    SculptMode::Mode mode = SculptMode::Pull;
    Curve falloff; // The falloff profile of the brush

    BrushSettings() {
        // Default to a smooth, linear falloff
        falloff.AddPoint({0.0f, 1.0f});
        falloff.AddPoint({1.0f, 0.0f});
    }
};