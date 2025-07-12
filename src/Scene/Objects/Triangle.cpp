#include "Scene/Objects/Triangle.h"

#include "Core/PropertyNames.h"
#include "Scene/Objects/BaseObject.h"
#include "Scene/Objects/ObjectTypes.h"

Triangle::Triangle() {
  name = std::string(ObjectTypes::Triangle);

  // A callback for when the mesh needs to be rebuilt
  auto onMeshChanged = [this]() { RebuildMesh(); };

  // A Triangle only has Width and Height. It no longer uses Depth.
  m_Properties.Add(PropertyNames::Width, 1.0f, onMeshChanged);
  m_Properties.Add(PropertyNames::Height, 1.0f, onMeshChanged);

  RebuildMesh();
}

std::string Triangle::GetTypeString() const {
  return std::string(ObjectTypes::Triangle);
}

glm::vec3 Triangle::GetLocalCenter() const {
  return glm::vec3(
      0.0f, m_Properties.GetValue<float>(PropertyNames::Height) * 0.5f, 0.0f);
}

std::vector<GizmoHandleDef> Triangle::GetGizmoHandleDefs() {
  if (!isPristine) {
    return BaseObject::GetGizmoHandleDefs();
  }
  return {{PropertyNames::Width, {1.0f, 0.0f, 0.0f}, {1, 0, 0, 1}},
          {PropertyNames::Height, {0.0f, 1.0f, 0.0f}, {0, 1, 0, 1}}};
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