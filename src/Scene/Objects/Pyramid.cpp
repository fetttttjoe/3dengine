#include "Scene/Objects/Pyramid.h"

#include "Core/PropertyNames.h"
#include "Scene/Objects/ObjectTypes.h"

Pyramid::Pyramid() {
  name = std::string(ObjectTypes::Pyramid);
  RebuildMesh();
}

std::string Pyramid::GetTypeString() const {
  return std::string(ObjectTypes::Pyramid);
}

glm::vec3 Pyramid::GetLocalCenter() const {
  return glm::vec3(
      0.0f, m_Properties.GetValue<float>(PropertyNames::Height) * 0.25f, 0.0f);
}

void Pyramid::BuildMeshData(std::vector<float>& vertices,
                            std::vector<unsigned int>& indices) {
  float w = m_Properties.GetValue<float>(PropertyNames::Width) * 0.5f;
  float h = m_Properties.GetValue<float>(PropertyNames::Height);
  float d = m_Properties.GetValue<float>(PropertyNames::Depth) * 0.5f;

  vertices = {
      // Base vertices (at y=0)
      -w, 0.0f, -d,  // 0
      w, 0.0f, -d,   // 1
      w, 0.0f, d,    // 2
      -w, 0.0f, d,   // 3
      // Apex vertex (at y=h)
      0, h, 0  // 4
  };

  indices = {// Base
             0, 1, 2, 2, 3, 0,
             // Sides
             0, 4, 1, 1, 4, 2, 2, 4, 3, 3, 4, 0};
}