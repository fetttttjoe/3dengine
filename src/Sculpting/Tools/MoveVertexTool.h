#pragma once

#include "Sculpting/ISculptTool.h"
#include <glm/glm.hpp>
#include <vector>

class SculptableMesh;
struct BrushSettings;

class MoveVertexTool : public ISculptTool {
 public:
  MoveVertexTool();

  void Apply(SculptableMesh& mesh, const glm::vec3& hitPoint,
             const glm::vec3& rayDirection, const glm::vec2& mouseDelta,
             const BrushSettings& settings, const glm::mat4& viewMatrix,
             const glm::mat4& projectionMatrix, int viewportWidth,
             int viewportHeight) override;

  void OnMouseDown(const SculptableMesh& mesh, const glm::vec3& rayOrigin,
                   const glm::vec3& rayDirection, // ray for context, but picking is screen-space now
                   const glm::vec2& mouseScreenPos, // NEW: Mouse position in screen coordinates
                   const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
                   int viewportWidth, int viewportHeight);
  
  void OnMouseDrag(SculptableMesh& mesh, const glm::vec2& mouseDelta,
                   const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
                   int viewportWidth, int viewportHeight);
  void OnMouseRelease(SculptableMesh& mesh);

  // FindClosestVertex now takes screen position for picking
  int FindClosestVertex(const SculptableMesh& mesh, const glm::vec2& mouseScreenPos, // NEW: Mouse position
                        const glm::mat4& viewProjMatrix, int viewportWidth, int viewportHeight,
                        float pickPixelThreshold) const; // NEW: Pixel threshold

  int GetSelectedVertexIndex() const { return m_SelectedVertexIndex; }
  bool IsDragging() const { return m_SelectedVertexIndex != -1; }

  void Reset();

 private:
  int m_SelectedVertexIndex = -1;
  glm::vec3 m_InitialVertexPos;
  float m_DragDepthNDC;
  glm::mat4 m_InitialViewProj;
};