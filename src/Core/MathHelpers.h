#pragma once

#include <glm/glm.hpp>
#include <string>    // For std::string
#include <sstream>   // For std::stringstream

#include "imgui.h"

// Provides simple conversion functions between ImGui's and GLM's vector types.
namespace MathHelpers {
inline glm::vec2 ToGlm(const ImVec2& v) { return glm::vec2(v.x, v.y); }

inline ImVec2 ToImGui(const glm::vec2& v) { return ImVec2(v.x, v.y); }

// Helper function to convert glm::vec2 to std::string
inline std::string ToString(const glm::vec2& v) {
    std::stringstream ss;
    ss << "vec2(" << v.x << ", " << v.y << ")";
    return ss.str();
}

// Helper function to convert glm::vec3 to std::string
inline std::string ToString(const glm::vec3& v) {
    std::stringstream ss;
    ss << "vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
    return ss.str();
}

// Helper function to convert glm::vec4 to std::string
inline std::string ToString(const glm::vec4& v) {
    std::stringstream ss;
    ss << "vec4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    return ss.str();
}

}  // namespace MathHelpers