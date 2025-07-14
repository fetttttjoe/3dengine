#pragma once

#include <glm/glm.hpp>
#include <unordered_set>
#include <vector>
#include <functional>
#include "Sculpting/ISculptTool.h"

class IEditableMesh;
class Camera;

// Custom hash for std::pair, used for storing edges
struct PairHash {
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2> &pair) const {
        return std::hash<T1>()(pair.first) ^ (std::hash<T2>()(pair.second) << 1);
    }
};

class SubObjectSelection {
 public:
  SubObjectSelection();

  void OnMouseDown(IEditableMesh& mesh, const Camera& camera, const glm::mat4& modelMatrix,
                   const glm::vec2& mouseScreenPos, int viewportWidth,
                   int viewportHeight, bool isShiftPressed, SubObjectMode mode);
  void OnMouseDrag(const glm::vec2& mouseDelta);
  void OnMouseRelease(IEditableMesh& mesh);

  void Clear();
  bool IsDragging() const;
  void ApplyDrag(IEditableMesh& mesh, const glm::mat4& viewMatrix,
                 const glm::mat4& projectionMatrix, int viewportWidth,
                 int viewportHeight);

  const std::unordered_set<uint32_t>& GetSelectedVertices() const;
  const std::unordered_set<std::pair<uint32_t, uint32_t>, PairHash>& GetSelectedEdges() const;
  const std::unordered_set<uint32_t>& GetSelectedFaces() const;
  const std::vector<std::pair<uint32_t, uint32_t>>& GetHighlightedPath() const;

  void SetIgnoreBackfaces(bool ignore) { m_IgnoreBackfaces = ignore; }
  bool GetIgnoreBackfaces() const { return m_IgnoreBackfaces; }

#if defined(INTUITIVE_MODELER_TESTING)
  // --- Test-only Helper Methods ---

  int FindClosestVertex_ForTests(const IEditableMesh& mesh, const glm::mat4& modelMatrix,
                                 const glm::vec2& mouseScreenPos,
                                 const glm::mat4& viewMatrix,
                                 const glm::mat4& projectionMatrix, const glm::vec3& cameraFwd,
                                 int viewportWidth, int viewportHeight, float pickPixelThreshold) const {
      return FindClosestVertex(mesh, modelMatrix, mouseScreenPos, viewMatrix,
                               projectionMatrix, cameraFwd, viewportWidth, viewportHeight, pickPixelThreshold);
  }

  std::pair<int, int> FindClosestEdge_ForTests(const IEditableMesh& mesh, const glm::mat4& modelMatrix,
                                 const glm::vec2& mouseScreenPos,
                                 const glm::mat4& viewMatrix,
                                 const glm::mat4& projectionMatrix, const glm::vec3& cameraFwd,
                                 int viewportWidth, int viewportHeight, float pickPixelThreshold) const {
      return FindClosestEdge(mesh, modelMatrix, mouseScreenPos, viewMatrix,
                             projectionMatrix, cameraFwd, viewportWidth, viewportHeight, pickPixelThreshold);
  }

  void SelectVertexForTest(uint32_t vertexIndex) { m_SelectedVertices.insert(vertexIndex); }
  void SelectFaceForTest(uint32_t faceIndex) { m_SelectedFaces.insert(faceIndex); }
#endif

 private:
  void FindShortestPath(IEditableMesh& mesh, uint32_t startNode, uint32_t endNode);
  int FindClosestVertex(const IEditableMesh& mesh, const glm::mat4& modelMatrix,
                        const glm::vec2& mouseScreenPos, const glm::mat4& viewMatrix,
                        const glm::mat4& projectionMatrix, const glm::vec3& cameraFwd,
                        int viewportWidth, int viewportHeight, float pickPixelThreshold) const;

  std::pair<int, int> FindClosestEdge(const IEditableMesh& mesh, const glm::mat4& modelMatrix,
                                      const glm::vec2& mouseScreenPos, const glm::mat4& viewMatrix,
                                      const glm::mat4& projectionMatrix, const glm::vec3& cameraFwd,
                                      int viewportWidth, int viewportHeight, float pickPixelThreshold) const;

  std::unordered_set<uint32_t> m_SelectedVertices;
  std::unordered_set<std::pair<uint32_t, uint32_t>, PairHash> m_SelectedEdges;
  std::unordered_set<uint32_t> m_SelectedFaces;
  std::vector<std::pair<uint32_t, uint32_t>> m_HighlightedPath;
  std::vector<uint32_t> m_SelectionOrder;
  
  bool m_IgnoreBackfaces = true;

  bool m_IsDragging = false;
  int m_ActiveDragVertexIndex = -1;
  glm::vec3 m_InitialDragPosition;
  float m_DragDepthNDC;
  glm::mat4 m_InitialViewProj;
  glm::mat4 m_ModelMatrix;
  glm::vec2 m_AccumulatedMouseDelta;
};