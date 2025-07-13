#include "Scene/Objects/Pyramid.h"

#include "Core/PropertyNames.h"
#include "Scene/Objects/BaseObject.h"
#include "Scene/Objects/ObjectTypes.h"

Pyramid::Pyramid() {
  name = std::string(ObjectTypes::Pyramid);
  auto onMeshChanged = [this]() { RebuildMesh(); };
  m_Properties.Add(PropertyNames::Width, 1.0f, onMeshChanged);
  m_Properties.Add(PropertyNames::Height, 1.0f, onMeshChanged);
  m_Properties.Add(PropertyNames::Depth, 1.0f, onMeshChanged);
  RebuildMesh();
}

std::string Pyramid::GetTypeString() const {
  return std::string(ObjectTypes::Pyramid);
}

glm::vec3 Pyramid::GetLocalCenter() const {
  return glm::vec3(
      0.0f, m_Properties.GetValue<float>(PropertyNames::Height) * 0.25f, 0.0f);
}

std::vector<GizmoHandleDef> Pyramid::GetGizmoHandleDefs() {
  if (!isPristine) {
    return BaseObject::GetGizmoHandleDefs();
  }
  return {{PropertyNames::Width, {1.0f, 0.0f, 0.0f}, {1, 0, 0, 1}},
          {PropertyNames::Height, {0.0f, 1.0f, 0.0f}, {0, 1, 0, 1}},
          {PropertyNames::Depth, {0.0f, 0.0f, 1.0f}, {0, 0, 1, 1}}};
}

void Pyramid::BuildMeshData(std::vector<float>& vertices,
                            std::vector<unsigned int>& indices) {
  float w = m_Properties.GetValue<float>(PropertyNames::Width) * 0.5f;
  float h = m_Properties.GetValue<float>(PropertyNames::Height);
  float d = m_Properties.GetValue<float>(PropertyNames::Depth) * 0.5f;

  vertices = {
      -w, 0.0f, -d,  // 0
      w,  0.0f, -d,  // 1
      w,  0.0f, d,   // 2
      -w, 0.0f, d,   // 3
      0,  h,    0    // 4
  };

  indices = {0, 1, 2, 2, 3, 0, 0, 4, 1, 1, 4, 2, 2, 4, 3, 3, 4, 0};
}