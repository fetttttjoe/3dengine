#pragma once

#include <glm/glm.hpp>

#include "imgui.h"

// Provides simple conversion functions between ImGui's and GLM's vector types.
namespace MathHelpers {
inline glm::vec2 ToGlm(const ImVec2& v) { return glm::vec2(v.x, v.y); }

inline ImVec2 ToImGui(const glm::vec2& v) { return ImVec2(v.x, v.y); }
}  // namespace MathHelpers