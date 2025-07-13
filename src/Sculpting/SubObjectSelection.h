#pragma once

#include <glm/glm.hpp>
#include <unordered_set>

#include "Core/Application.h"

class IEditableMesh;

class SubObjectSelection {
 public:
  SubObjectSelection();

  void OnMouseDown(IEditableMesh& mesh, const glm::vec3& rayOrigin,
                   const glm::vec3& rayDirection, const glm::mat4& modelMatrix,
                   const glm::vec2& mouseScreenPos, const glm::mat4& viewMatrix,
                   const glm::mat4& projectionMatrix, int viewportWidth,
                   int viewportHeight, bool isShiftPressed, SubObjectMode mode);
  void OnMouseDrag(const glm::vec2& mouseDelta);
  void OnMouseRelease(IEditableMesh& mesh);

  void Clear();
  bool IsDragging() const;
  void ApplyDrag(IEditableMesh& mesh, const glm::mat4& viewMatrix,
                 const glm::mat4& projectionMatrix, int viewportWidth,
                 int viewportHeight);

  const std::unordered_set<uint32_t>& GetSelectedVertices() const;
  const std::unordered_set<uint32_t>& GetSelectedFaces() const;

 private:
  int FindClosestVertex(const IEditableMesh& mesh, const glm::mat4& modelMatrix,
                        const glm::vec2& mouseScreenPos,
                        const glm::mat4& viewMatrix,
                        const glm::mat4& projectionMatrix, int viewportWidth,
                        int viewportHeight, float pickPixelThreshold) const;

  std::unordered_set<uint32_t> m_SelectedVertices;
  std::unordered_set<uint32_t> m_SelectedFaces;

  bool m_IsDragging = false;
  int m_ActiveDragVertexIndex = -1;
  glm::vec3 m_InitialDragPosition;
  float m_DragDepthNDC;
  glm::mat4 m_InitialViewProj;
  glm::mat4 m_ModelMatrix;
  glm::vec2 m_AccumulatedMouseDelta;
};