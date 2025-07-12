#include "Scene/Objects/Icosphere.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "Core/PropertyNames.h"
#include "Scene/Objects/BaseObject.h"
#include "Scene/Objects/ObjectTypes.h"

Icosphere::Icosphere() {
  name = std::string(ObjectTypes::Icosphere);
  m_Properties.Add(PropertyNames::Radius, 1.0f, [this]() { RebuildMesh(); });
  RebuildMesh();
}

std::string Icosphere::GetTypeString() const {
  return std::string(ObjectTypes::Icosphere);
}

std::vector<GizmoHandleDef> Icosphere::GetGizmoHandleDefs() {
  return BaseObject::GetGizmoHandleDefs();
}

// The OnGizmoUpdate function is correctly removed from this file.
// The class now inherits the desired scaling logic from ScalableSphereObject.

// The essential implementation for getMiddlePoint remains.
int Icosphere::getMiddlePoint(int p1, int p2, std::vector<glm::vec3>& vertices,
                              std::map<int64_t, int>& middlePointIndexCache) {
  bool firstIsSmaller = p1 < p2;
  int64_t smallerIndex = firstIsSmaller ? p1 : p2;
  int64_t greaterIndex = firstIsSmaller ? p2 : p1;
  int64_t key = (smallerIndex << 32) + greaterIndex;

  auto it = middlePointIndexCache.find(key);
  if (it != middlePointIndexCache.end()) {
    return it->second;
  }

  glm::vec3 point1 = vertices[p1];
  glm::vec3 point2 = vertices[p2];
  glm::vec3 middle =
      glm::vec3((point1.x + point2.x) / 2.0f, (point1.y + point2.y) / 2.0f,
                (point1.z + point2.z) / 2.0f);

  vertices.push_back(glm::normalize(middle));
  int i = vertices.size() - 1;

  middlePointIndexCache[key] = i;
  return i;
}

void Icosphere::BuildMeshData(std::vector<float>& outVertices,
                              std::vector<unsigned int>& outIndices) {
  std::vector<glm::vec3> positions;
  std::map<int64_t, int> middlePointIndexCache;
  float radius = m_Properties.GetValue<float>(PropertyNames::Radius);

  float t = (1.0f + sqrt(5.0f)) / 2.0f;

  positions.push_back(glm::normalize(glm::vec3(-1, t, 0)));
  positions.push_back(glm::normalize(glm::vec3(1, t, 0)));
  positions.push_back(glm::normalize(glm::vec3(-1, -t, 0)));
  positions.push_back(glm::normalize(glm::vec3(1, -t, 0)));
  positions.push_back(glm::normalize(glm::vec3(0, -1, t)));
  positions.push_back(glm::normalize(glm::vec3(0, 1, t)));
  positions.push_back(glm::normalize(glm::vec3(0, -1, -t)));
  positions.push_back(glm::normalize(glm::vec3(0, 1, -t)));
  positions.push_back(glm::normalize(glm::vec3(t, 0, -1)));
  positions.push_back(glm::normalize(glm::vec3(t, 0, 1)));
  positions.push_back(glm::normalize(glm::vec3(-t, 0, -1)));
  positions.push_back(glm::normalize(glm::vec3(-t, 0, 1)));

  std::vector<glm::ivec3> faces;
  faces.push_back({0, 11, 5});
  faces.push_back({0, 5, 1});
  faces.push_back({0, 1, 7});
  faces.push_back({0, 7, 10});
  faces.push_back({0, 10, 11});
  faces.push_back({1, 5, 9});
  faces.push_back({5, 11, 4});
  faces.push_back({11, 10, 2});
  faces.push_back({10, 7, 6});
  faces.push_back({7, 1, 8});
  faces.push_back({3, 9, 4});
  faces.push_back({3, 4, 2});
  faces.push_back({3, 2, 6});
  faces.push_back({3, 6, 8});
  faces.push_back({3, 8, 9});
  faces.push_back({4, 9, 5});
  faces.push_back({2, 4, 11});
  faces.push_back({6, 2, 10});
  faces.push_back({8, 6, 7});
  faces.push_back({9, 8, 1});

  for (int i = 0; i < m_RecursionLevel; i++) {
    std::vector<glm::ivec3> faces2;
    for (auto& tri : faces) {
      int a = getMiddlePoint(tri.x, tri.y, positions, middlePointIndexCache);
      int b = getMiddlePoint(tri.y, tri.z, positions, middlePointIndexCache);
      int c = getMiddlePoint(tri.z, tri.x, positions, middlePointIndexCache);

      faces2.push_back({tri.x, a, c});
      faces2.push_back({tri.y, b, a});
      faces2.push_back({tri.z, c, b});
      faces2.push_back({a, b, c});
    }
    faces = faces2;
  }

  outVertices.clear();
  outIndices.clear();
  for (const auto& pos : positions) {
    outVertices.push_back(pos.x * radius);
    outVertices.push_back(pos.y * radius);
    outVertices.push_back(pos.z * radius);
  }
  for (const auto& face : faces) {
    outIndices.push_back(face.x);
    outIndices.push_back(face.y);
    outIndices.push_back(face.z);
  }
}