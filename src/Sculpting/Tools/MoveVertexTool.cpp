#include "Sculpting/Tools/MoveVertexTool.h"
#include "Sculpting/SculptableMesh.h"
#include "Core/Camera.h" // For Camera::WorldToScreen, ScreenToWorldPoint, ScreenToWorldRay
#include "Core/Log.h"
#include "Core/MathHelpers.h" // For ToString (and ToGlm if needed for internal calculations)
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/norm.hpp> // For glm::distance2
#include <limits> // For std::numeric_limits
#include <iostream> // For std::ostream (for the operator<< overload)


MoveVertexTool::MoveVertexTool() {
    Reset();
}

void MoveVertexTool::Reset() {
    m_SelectedVertexIndex = -1;
    m_InitialVertexPos = glm::vec3(0.0f);
    m_DragDepthNDC = 0.0f;
    m_InitialViewProj = glm::mat4(1.0f);
    Log::Debug("MoveVertexTool: Tool state reset.");
}

void MoveVertexTool::Apply(SculptableMesh& mesh, const glm::vec3& hitPoint,
                           const glm::vec3& rayDirection, const glm::vec2& mouseDelta,
                           const BrushSettings& settings, const glm::mat4& viewMatrix,
                           const glm::mat4& projectionMatrix, int viewportWidth,
                           int viewportHeight) {
    if (m_SelectedVertexIndex != -1 && glm::length(mouseDelta) > 0.0001f) {
        OnMouseDrag(mesh, mouseDelta, viewMatrix, projectionMatrix, viewportWidth, viewportHeight);
    }
}


void MoveVertexTool::OnMouseDown(const SculptableMesh& mesh, const glm::vec3& rayOrigin,
                               const glm::vec3& rayDirection, // rayDirection is now contextual for logging, not primary for picking
                               const glm::vec2& mouseScreenPos, // NEW: Mouse screen position
                               const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
                               int viewportWidth, int viewportHeight) {
    Log::Debug("MoveVertexTool: OnMouseDown received. Attempting to select vertex.");
    
    // Define a pixel-based threshold for picking
    // This is the radius in pixels around the mouse cursor to detect a vertex
    float pickPixelThreshold = 8.0f; // Adjust this value as needed (e.g., 5.0f to 10.0f pixels)

    glm::mat4 currentViewProj = projectionMatrix * viewMatrix;

    // Use screen-space picking
    m_SelectedVertexIndex = FindClosestVertex(mesh, mouseScreenPos,
                                              currentViewProj, viewportWidth, viewportHeight,
                                              pickPixelThreshold);

    if (m_SelectedVertexIndex != -1) {
        m_InitialVertexPos = mesh.m_Vertices[m_SelectedVertexIndex];
        
        // Calculate initial depth and store matrices for consistent dragging plane
        glm::vec4 clipPos = currentViewProj * glm::vec4(m_InitialVertexPos, 1.0f);
        m_DragDepthNDC = (clipPos.w != 0.0f) ? (clipPos.z / clipPos.w) : 0.0f;
        m_InitialViewProj = currentViewProj; // Store this for consistent dragging plane

        Log::Debug("MoveVertexTool: Selected vertex ", m_SelectedVertexIndex, " at ", MathHelpers::ToString(m_InitialVertexPos), " (Screen: ", MathHelpers::ToString(Camera::WorldToScreen(m_InitialVertexPos, currentViewProj, viewportWidth, viewportHeight)), ") with initial depth ", m_DragDepthNDC);
    } else {
        Log::Debug("MoveVertexTool: No vertex found under cursor (screenPos: ", MathHelpers::ToString(mouseScreenPos), ") on mouse down with pixel threshold ", pickPixelThreshold, ".");
    }
}

void MoveVertexTool::OnMouseDrag(SculptableMesh& mesh, const glm::vec2& mouseDelta,
                                 const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
                                 int viewportWidth, int viewportHeight) {
    if (m_SelectedVertexIndex == -1) {
        return; // No vertex selected to drag
    }
    if (glm::length(mouseDelta) == 0.0f) {
        return; // No actual mouse movement this frame
    }

    // Calculate initial vertex screen position based on where the *initial* vertex was on screen
    // and the matrices *at the moment of selection* (m_InitialViewProj)
    glm::vec2 initialVertexScreenPos = Camera::WorldToScreen(
        m_InitialVertexPos, m_InitialViewProj, viewportWidth, viewportHeight);
    
    // Add the current frame's mouse delta to get the new screen position
    glm::vec2 currentMouseScreenPos = initialVertexScreenPos + mouseDelta;

    // Unproject the new screen position back to world space at the *initial drag depth*
    glm::mat4 invInitialViewProj = glm::inverse(m_InitialViewProj); // Use inverse of the stored matrix
    glm::vec3 newWorldPos = Camera::ScreenToWorldPoint(
        currentMouseScreenPos, m_DragDepthNDC, invInitialViewProj, viewportWidth, viewportHeight
    );

    mesh.m_Vertices[m_SelectedVertexIndex] = newWorldPos;
}

void MoveVertexTool::OnMouseRelease(SculptableMesh& mesh) {
    if (m_SelectedVertexIndex != -1) {
        Log::Debug("MoveVertexTool: OnMouseRelease received. Finalizing drag for vertex ", m_SelectedVertexIndex, " at final pos ", MathHelpers::ToString(mesh.m_Vertices[m_SelectedVertexIndex]));
        mesh.RecalculateNormals(); // Recalculate normals once after final position is set
    } else {
        Log::Debug("MoveVertexTool: OnMouseRelease received, but no vertex was active.");
    }
    Reset(); // Always reset state after release
}

// NEW: Screen-space picking implementation
int MoveVertexTool::FindClosestVertex(const SculptableMesh& mesh, const glm::vec2& mouseScreenPos,
                                    const glm::mat4& viewProjMatrix, int viewportWidth, int viewportHeight,
                                    float pickPixelThreshold) const {
    int closestIndex = -1;
    float minDistanceSq = pickPixelThreshold * pickPixelThreshold; // Squared threshold for efficiency

    const auto& vertices = mesh.GetVertices();
    for (size_t i = 0; i < vertices.size(); ++i) {
        const glm::vec3& vertexWorldPos = vertices[i];

        // 1. Project vertex to screen coordinates using the actual Camera helper
        glm::vec2 vertexScreenPos = Camera::WorldToScreen(
            vertexWorldPos, viewProjMatrix, viewportWidth, viewportHeight);

        // 2. Calculate squared 2D pixel distance to mouse cursor
        float distSq = glm::distance2(mouseScreenPos, vertexScreenPos);

        // 3. Check if it's within the pixel threshold and closer than previous hits
        if (distSq < minDistanceSq) {
            minDistanceSq = distSq;
            closestIndex = static_cast<int>(i);
        }
    }
    Log::Debug("MoveVertexTool::FindClosestVertex: Mouse screen pos: ", MathHelpers::ToString(mouseScreenPos), ", Pick threshold: ", pickPixelThreshold, ". Closest index found: ", closestIndex, ", Min Dist Sq: ", minDistanceSq);
    return closestIndex;
}