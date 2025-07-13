#include "Sculpting/SubObjectSelection.h"

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/norm.hpp>

#include "Core/Log.h"
#include "Core/MathHelpers.h"  // CORRECT HEADER
#include "Core/Raycaster.h"
#include "Interfaces/IEditableMesh.h"

SubObjectSelection::SubObjectSelection() { Clear(); }

void SubObjectSelection::Clear() {
  m_SelectedVertices.clear();
  m_SelectedFaces.clear();
  m_IsDragging = false;
  m_ActiveDragVertexIndex = -1;
}

bool SubObjectSelection::IsDragging() const { return m_IsDragging; }

const std::unordered_set<uint32_t>& SubObjectSelection::GetSelectedVertices()
    const {
  return m_SelectedVertices;
}

const std::unordered_set<uint32_t>& SubObjectSelection::GetSelectedFaces()
    const {
  return m_SelectedFaces;
}

void SubObjectSelection::OnMouseDown(
    IEditableMesh& mesh, const glm::vec3& rayOrigin,
    const glm::vec3& rayDirection, const glm::mat4& modelMatrix,
    const glm::vec2& mouseScreenPos, const glm::mat4& viewMatrix,
    const glm::mat4& projectionMatrix, int viewportWidth, int viewportHeight,
    bool isShiftPressed, SubObjectMode mode) {
  m_IsDragging = false;
  m_AccumulatedMouseDelta = glm::vec2(0.0f);
  m_InitialViewProj = projectionMatrix * viewMatrix;
  m_ModelMatrix = modelMatrix;

  if (mode == SubObjectMode::VERTEX) {
    int closestIndex = FindClosestVertex(mesh, modelMatrix, mouseScreenPos,
                                         viewMatrix, projectionMatrix,
                                         viewportWidth, viewportHeight, 15.0f);

    if (closestIndex != -1) {
      m_IsDragging = true;
      if (!isShiftPressed) {
        m_SelectedVertices.clear();
      }
      if (m_SelectedVertices.count(closestIndex)) {
        m_SelectedVertices.erase(closestIndex);
      } else {
        m_SelectedVertices.insert(closestIndex);
      }
      m_ActiveDragVertexIndex = closestIndex;

      glm::vec3 vertexWorldPos = glm::vec3(
          modelMatrix *
          glm::vec4(mesh.GetVertices()[m_ActiveDragVertexIndex], 1.0f));
      m_InitialDragPosition = vertexWorldPos;

      glm::vec4 clipPos =
          m_InitialViewProj * glm::vec4(m_InitialDragPosition, 1.0f);
      m_DragDepthNDC = clipPos.w != 0.0f ? clipPos.z / clipPos.w : 0.0f;

    } else {
      if (!isShiftPressed) m_SelectedVertices.clear();
      m_ActiveDragVertexIndex = -1;
    }
  } else if (mode == SubObjectMode::FACE) {
    Raycaster::RaycastResult result;
    if (Raycaster::IntersectMesh(rayOrigin, rayDirection, mesh, modelMatrix,
                                 result) &&
        result.triangleIndex != -1) {
      if (!isShiftPressed) {
        m_SelectedFaces.clear();
      }
      if (m_SelectedFaces.count(result.triangleIndex)) {
        m_SelectedFaces.erase(result.triangleIndex);
      } else {
        m_SelectedFaces.insert(result.triangleIndex);
      }
    } else {
      if (!isShiftPressed) m_SelectedFaces.clear();
    }
  }
}

void SubObjectSelection::OnMouseDrag(const glm::vec2& mouseDelta) {
  if (m_IsDragging && m_ActiveDragVertexIndex != -1) {
    m_AccumulatedMouseDelta += mouseDelta;
  }
}

void SubObjectSelection::OnMouseRelease(IEditableMesh& mesh) {
  if (m_IsDragging && m_ActiveDragVertexIndex != -1) {
    mesh.RecalculateNormals();
  }
  m_IsDragging = false;
  m_ActiveDragVertexIndex = -1;
  m_AccumulatedMouseDelta = glm::vec2(0.0f);
}

void SubObjectSelection::ApplyDrag(IEditableMesh& mesh,
                                   const glm::mat4& viewMatrix,
                                   const glm::mat4& projectionMatrix,
                                   int viewportWidth, int viewportHeight) {
  if (!m_IsDragging || m_ActiveDragVertexIndex == -1) return;

  glm::mat4 invInitialViewProj = glm::inverse(m_InitialViewProj);

  // CORRECTED: Use MathHelpers instead of Camera
  glm::vec2 initialScreenPos = MathHelpers::WorldToScreen(
      m_InitialDragPosition, m_InitialViewProj, viewportWidth, viewportHeight);
  glm::vec2 newScreenPos = initialScreenPos + m_AccumulatedMouseDelta;

  // CORRECTED: Use MathHelpers instead of Camera
  glm::vec3 newWorldPos = MathHelpers::ScreenToWorldPoint(
      newScreenPos, m_DragDepthNDC, invInitialViewProj, viewportWidth,
      viewportHeight);

  glm::vec3 worldSpaceDelta = newWorldPos - m_InitialDragPosition;

  glm::mat4 invModel = glm::inverse(m_ModelMatrix);
  glm::vec3 localSpaceDelta =
      glm::vec3(invModel * glm::vec4(worldSpaceDelta, 0.0f));

  for (uint32_t index : m_SelectedVertices) {
    mesh.GetVertices()[index] += localSpaceDelta;
  }
}

int SubObjectSelection::FindClosestVertex(const IEditableMesh& mesh,
                                          const glm::mat4& modelMatrix,
                                          const glm::vec2& mouseScreenPos,
                                          const glm::mat4& viewMatrix,
                                          const glm::mat4& projectionMatrix,
                                          int viewportWidth, int viewportHeight,
                                          float pickPixelThreshold) const {
  int closestIndex = -1;
  float minDistanceSq = pickPixelThreshold * pickPixelThreshold;
  const auto& vertices = mesh.GetVertices();

  glm::mat4 viewProjMatrix = projectionMatrix * viewMatrix;

  for (size_t i = 0; i < vertices.size(); ++i) {
    glm::vec3 worldPos = glm::vec3(modelMatrix * glm::vec4(vertices[i], 1.0f));
    // CORRECTED: Use MathHelpers instead of Camera
    glm::vec2 vertexScreenPos = MathHelpers::WorldToScreen(
        worldPos, viewProjMatrix, viewportWidth, viewportHeight);

    if (vertexScreenPos.x < 0) continue;

    float distSq = glm::distance2(mouseScreenPos, vertexScreenPos);

    if (distSq < minDistanceSq) {
      minDistanceSq = distSq;
      closestIndex = static_cast<int>(i);
    }
  }
  return closestIndex;
}