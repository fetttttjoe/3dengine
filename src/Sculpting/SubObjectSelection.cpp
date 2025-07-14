#include "Sculpting/SubObjectSelection.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/norm.hpp>
#include <set>
#include <queue>
#include <map>
#include <algorithm>
#include "Core/Log.h"
#include "Core/MathHelpers.h"
#include "Core/Raycaster.h"
#include "Interfaces/IEditableMesh.h"
#include "Core/Camera.h"

float PointToSegmentDistance(const glm::vec2& p, const glm::vec2& a, const glm::vec2& b) {
    glm::vec2 ab = b - a;
    glm::vec2 ap = p - a;
    float l2 = glm::dot(ab, ab);
    if (l2 == 0.0f) return glm::distance(p, a);
    float t = std::max(0.0f, std::min(1.0f, glm::dot(ap, ab) / l2));
    glm::vec2 projection = a + t * ab;
    return glm::distance(p, projection);
}

SubObjectSelection::SubObjectSelection() { Clear(); }

void SubObjectSelection::Clear() {
  m_SelectedVertices.clear();
  m_SelectedEdges.clear();
  m_SelectedFaces.clear();
  m_HighlightedPath.clear();
  m_SelectionOrder.clear();
  m_IsDragging = false;
  m_ActiveDragVertexIndex = -1;
}

bool SubObjectSelection::IsDragging() const { return m_IsDragging; }

const std::unordered_set<uint32_t>& SubObjectSelection::GetSelectedVertices() const { return m_SelectedVertices; }
const std::unordered_set<std::pair<uint32_t, uint32_t>, PairHash>& SubObjectSelection::GetSelectedEdges() const { return m_SelectedEdges; }
const std::unordered_set<uint32_t>& SubObjectSelection::GetSelectedFaces() const { return m_SelectedFaces; }
const std::vector<std::pair<uint32_t, uint32_t>>& SubObjectSelection::GetHighlightedPath() const { return m_HighlightedPath; }

void SubObjectSelection::OnMouseDown(IEditableMesh& mesh, const Camera& camera, const glm::mat4& modelMatrix, const glm::vec2& mouseScreenPos, int viewportWidth, int viewportHeight, bool isShiftPressed, SubObjectMode mode) {
    m_IsDragging = false;
    m_AccumulatedMouseDelta = glm::vec2(0.0f);
    m_InitialViewProj = camera.GetProjectionMatrix() * camera.GetViewMatrix();
    m_ModelMatrix = modelMatrix;
    
    if (!isShiftPressed) {
        Clear();
    }

    glm::mat4 viewMatrix = camera.GetViewMatrix();
    glm::mat4 projectionMatrix = camera.GetProjectionMatrix();
    glm::vec3 cameraFwd = camera.GetFront();

    if (mode == SubObjectMode::VERTEX) {
        int closestIndex = FindClosestVertex(mesh, modelMatrix, mouseScreenPos, viewMatrix, projectionMatrix, cameraFwd, viewportWidth, viewportHeight, 15.0f);
        if (closestIndex != -1) {
            m_IsDragging = true;
            if (m_SelectedVertices.count(closestIndex)) {
                m_SelectedVertices.erase(closestIndex);
                m_SelectionOrder.erase(std::remove(m_SelectionOrder.begin(), m_SelectionOrder.end(), closestIndex), m_SelectionOrder.end());
                m_HighlightedPath.clear();
                if(m_SelectionOrder.size() >= 2) {
                    for(size_t i = 0; i < m_SelectionOrder.size() - 1; ++i) {
                        FindShortestPath(mesh, m_SelectionOrder[i], m_SelectionOrder[i+1]);
                    }
                }
            }
            else {
                m_SelectedVertices.insert(closestIndex);
                if (m_SelectionOrder.size() >= 1 && isShiftPressed) {
                    FindShortestPath(mesh, m_SelectionOrder.back(), closestIndex);
                }
                m_SelectionOrder.push_back(closestIndex);
            }
            m_ActiveDragVertexIndex = closestIndex;
            
            glm::vec3 vertexWorldPos = glm::vec3(modelMatrix * glm::vec4(mesh.GetVertices()[m_ActiveDragVertexIndex], 1.0f));
            m_InitialDragPosition = vertexWorldPos;
            glm::vec4 clipPos = m_InitialViewProj * glm::vec4(m_InitialDragPosition, 1.0f);
            m_DragDepthNDC = clipPos.w != 0.0f ? clipPos.z / clipPos.w : 0.0f;
        }
    } else if (mode == SubObjectMode::EDGE) {
        std::pair<int, int> closestEdge = FindClosestEdge(mesh, modelMatrix, mouseScreenPos, viewMatrix, projectionMatrix, cameraFwd, viewportWidth, viewportHeight, 10.0f);
        if (closestEdge.first != -1) {
            Log::Debug("Edge selected: (", closestEdge.first, ", ", closestEdge.second, ")");
            std::pair<uint32_t, uint32_t> edgeKey = { (uint32_t)closestEdge.first, (uint32_t)closestEdge.second };
            if (m_SelectedEdges.count(edgeKey)) m_SelectedEdges.erase(edgeKey);
            else m_SelectedEdges.insert(edgeKey);
        }
    } else if (mode == SubObjectMode::FACE) {
        glm::vec3 rayOrigin = camera.GetPosition();
        glm::vec3 rayDirection = camera.ScreenToWorldRay(mouseScreenPos, viewportWidth, viewportHeight);
        Raycaster::RaycastResult result;
        if (Raycaster::IntersectMesh(rayOrigin, rayDirection, mesh, modelMatrix, result) && result.triangleIndex != -1) {
            if (m_IgnoreBackfaces) {
                const auto& normals = mesh.GetNormals();
                glm::vec3 faceNormal = (normals[mesh.GetIndices()[result.triangleIndex * 3]] +
                                        normals[mesh.GetIndices()[result.triangleIndex * 3 + 1]] +
                                        normals[mesh.GetIndices()[result.triangleIndex * 3 + 2]]) / 3.0f;
                faceNormal = glm::normalize(glm::mat3(glm::transpose(glm::inverse(modelMatrix))) * faceNormal);
                if (glm::dot(faceNormal, -rayDirection) < 0.05f) return;
            }

            if (m_SelectedFaces.count(result.triangleIndex)) m_SelectedFaces.erase(result.triangleIndex);
            else m_SelectedFaces.insert(result.triangleIndex);
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

void SubObjectSelection::ApplyDrag(IEditableMesh& mesh, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, int viewportWidth, int viewportHeight) {
  if (!m_IsDragging || m_ActiveDragVertexIndex == -1 || glm::length(m_AccumulatedMouseDelta) == 0.0f) return;

  glm::mat4 invViewMatrix = glm::inverse(viewMatrix);
  glm::vec3 right = glm::vec3(invViewMatrix[0]);
  glm::vec3 up = glm::vec3(invViewMatrix[1]);
  glm::vec3 worldSpaceDelta = (right * m_AccumulatedMouseDelta.x - up * m_AccumulatedMouseDelta.y) * 0.01f;

  glm::mat4 invModel = glm::inverse(m_ModelMatrix);
  glm::vec3 localSpaceDelta = glm::vec3(invModel * glm::vec4(worldSpaceDelta, 0.0f));

  for (uint32_t index : m_SelectedVertices) {
    mesh.GetVertices()[index] += localSpaceDelta;
  }

  m_AccumulatedMouseDelta = glm::vec2(0.0f);
}

void SubObjectSelection::FindShortestPath(IEditableMesh& mesh, uint32_t startNode, uint32_t endNode) {
    // Build adjacency list
    std::map<uint32_t, std::vector<uint32_t>> adj;
    const auto& indices = mesh.GetIndices();
    for (size_t i = 0; i < indices.size(); i += 3) {
        uint32_t v0 = indices[i];
        uint32_t v1 = indices[i + 1];
        uint32_t v2 = indices[i + 2];
        adj[v0].push_back(v1); adj[v0].push_back(v2);
        adj[v1].push_back(v0); adj[v1].push_back(v2);
        adj[v2].push_back(v0); adj[v2].push_back(v1);
    }

    std::queue<uint32_t> q;
    std::map<uint32_t, uint32_t> parent;
    std::map<uint32_t, bool> visited;

    q.push(startNode);
    visited[startNode] = true;

    bool found = false;
    while (!q.empty()) {
        uint32_t u = q.front();
        q.pop();

        if (u == endNode) {
            found = true;
            break;
        }

        for (uint32_t v : adj[u]) {
            if (!visited[v]) {
                visited[v] = true;
                parent[v] = u;
                q.push(v);
            }
        }
    }

    if (found) {
        uint32_t crawl = endNode;
        while (crawl != startNode) {
            m_HighlightedPath.push_back({crawl, parent[crawl]});
            crawl = parent[crawl];
        }
    }
}

int SubObjectSelection::FindClosestVertex(const IEditableMesh& mesh, const glm::mat4& modelMatrix, const glm::vec2& mouseScreenPos, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const glm::vec3& cameraFwd, int viewportWidth, int viewportHeight, float pickPixelThreshold) const {
  int closestIndex = -1;
  float minDistanceSq = pickPixelThreshold * pickPixelThreshold;
  const auto& vertices = mesh.GetVertices();
  const auto& normals = mesh.GetNormals();
  glm::mat4 viewProjMatrix = projectionMatrix * viewMatrix;

  for (size_t i = 0; i < vertices.size(); ++i) {
    if (m_IgnoreBackfaces && i < normals.size()) {
        glm::vec3 normalWorld = glm::normalize(glm::mat3(glm::transpose(glm::inverse(modelMatrix))) * normals[i]);
        if (glm::dot(normalWorld, -cameraFwd) < 0.1f) continue;
    }
    glm::vec3 worldPos = glm::vec3(modelMatrix * glm::vec4(vertices[i], 1.0f));
    glm::vec2 vertexScreenPos = MathHelpers::WorldToScreen(worldPos, viewProjMatrix, viewportWidth, viewportHeight);
    if (vertexScreenPos.x < 0) continue;
    float distSq = glm::distance2(mouseScreenPos, vertexScreenPos);
    if (distSq < minDistanceSq) {
      minDistanceSq = distSq;
      closestIndex = static_cast<int>(i);
    }
  }
  return closestIndex;
}

std::pair<int, int> SubObjectSelection::FindClosestEdge(const IEditableMesh& mesh, const glm::mat4& modelMatrix, const glm::vec2& mouseScreenPos, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const glm::vec3& cameraFwd, int viewportWidth, int viewportHeight, float pickPixelThreshold) const {
    std::pair<int, int> closestEdge = {-1, -1};
    float minDistance = pickPixelThreshold;
    const auto& vertices = mesh.GetVertices();
    const auto& indices = mesh.GetIndices();
    const auto& normals = mesh.GetNormals();
    glm::mat4 viewProjMatrix = projectionMatrix * viewMatrix;

    std::set<std::pair<uint32_t, uint32_t>> uniqueEdges;
    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        uint32_t i0 = indices[i], i1 = indices[i+1], i2 = indices[i+2];
        uniqueEdges.insert({std::min(i0, i1), std::max(i0, i1)});
        uniqueEdges.insert({std::min(i1, i2), std::max(i1, i2)});
        uniqueEdges.insert({std::min(i2, i0), std::max(i2, i0)});
    }

    for (const auto& edge : uniqueEdges) {
        if (edge.first >= vertices.size() || edge.second >= vertices.size()) continue;

        if (m_IgnoreBackfaces) {
            if (edge.first < normals.size() && edge.second < normals.size()) {
                glm::vec3 avgNormal = glm::normalize(normals[edge.first] + normals[edge.second]);
                glm::vec3 normalWorld = glm::normalize(glm::mat3(glm::transpose(glm::inverse(modelMatrix))) * avgNormal);
                if (glm::dot(normalWorld, -cameraFwd) < 0.1f) continue;
            }
        }

        glm::vec3 v0_world = glm::vec3(modelMatrix * glm::vec4(vertices[edge.first], 1.0f));
        glm::vec3 v1_world = glm::vec3(modelMatrix * glm::vec4(vertices[edge.second], 1.0f));
        glm::vec2 v0_screen = MathHelpers::WorldToScreen(v0_world, viewProjMatrix, viewportWidth, viewportHeight);
        glm::vec2 v1_screen = MathHelpers::WorldToScreen(v1_world, viewProjMatrix, viewportWidth, viewportHeight);
        if(v0_screen.x < 0 || v1_screen.x < 0) continue;

        float distance = PointToSegmentDistance(mouseScreenPos, v0_screen, v1_screen);
        if (distance < minDistance) {
            minDistance = distance;
            closestEdge = edge;
        }
    }
    return closestEdge;
}