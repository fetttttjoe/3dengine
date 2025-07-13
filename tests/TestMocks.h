#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // For glm::lookAt, glm::ortho
#include <glm/gtc/matrix_inverse.hpp>   // For glm::inverse
#include <imgui.h> // Required for ImVec2, ImGuiMouseButton_

// --- Mock ImGuiIO for testing ---
// These remain inline because they are simple structs/getters.
namespace MockImGui {
    struct MockImGuiIO {
        ImVec2 MouseDelta;
    };
    inline MockImGuiIO g_MockIO; // inline allows definition in multiple CUs without error

    inline MockImGuiIO& GetIO() { return g_MockIO; }
} // namespace MockImGui


// --- Mock Camera functions for testing ---
// DECLARE them here as extern, so all test .cpp files know they exist.
namespace MockCamera {
    // Declare static variables as extern, they will be defined once in TestMocks.cpp
    extern glm::vec3 s_CameraPosition;
    extern glm::mat4 s_ViewMatrix;
    extern glm::mat4 s_ProjectionMatrix;

    // Declare functions, their definitions will be in TestMocks.cpp
    glm::vec3 GetPosition();

    glm::vec2 WorldToScreen(const glm::vec3& worldPos,
                                   const glm::mat4& viewProj, int windowWidth,
                                   int windowHeight);

    glm::vec3 ScreenToWorldPoint(const glm::vec2& screenPos, float ndcZ,
                                        const glm::mat4& invViewProj,
                                        int windowWidth, int windowHeight);
    
    glm::vec3 ScreenToWorldRay(const glm::vec2& screenPos, int windowWidth, int windowHeight);
} // namespace MockCamera