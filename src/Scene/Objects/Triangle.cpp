#include "Scene/Objects/Triangle.h"

#include "Core/PropertyNames.h"
#include "Scene/Objects/ObjectTypes.h"

Triangle::Triangle() {
  name = std::string(ObjectTypes::Triangle);
  m_Properties.SetValue<float>(PropertyNames::Depth, 0.0f);
  RebuildMesh();
}

std::string Triangle::GetTypeString() const {
  return std::string(ObjectTypes::Triangle);
}

glm::vec3 Triangle::GetLocalCenter() const {
  return glm::vec3(
      0.0f, m_Properties.GetValue<float>(PropertyNames::Height) * 0.5f, 0.0f);
}

void Triangle::BuildMeshData(std::vector<float>& vertices,
                             std::vector<unsigned int>& indices) {
  float half_w = m_Properties.GetValue<float>(PropertyNames::Width) * 0.5f;
  float h = m_Properties.GetValue<float>(PropertyNames::Height);

  vertices = {
      0.0f, h, 0.0f, -half_w, 0.0f, 0.0f, half_w, 0.0f, 0.0f,
  };

  indices = {0, 1, 2};
}