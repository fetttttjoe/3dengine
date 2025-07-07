#include "Scene/Objects/Sphere.h"

#include <cmath>
#include <glm/gtx/component_wise.hpp>

#include "Core/PropertyNames.h"
#include "Scene/Objects/ObjectTypes.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Sphere::Sphere() {
  name = std::string(ObjectTypes::Sphere);
  m_Properties = PropertySet();
  auto onTransformChanged = [this]() { m_IsTransformDirty = true; };
  m_Properties.Add(PropertyNames::Position, glm::vec3(0.0f),
                   onTransformChanged);
  m_Properties.Add(PropertyNames::Rotation, glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                   onTransformChanged);
  m_Properties.Add(PropertyNames::Scale, glm::vec3(1.0f), onTransformChanged);
  m_Properties.Add(PropertyNames::Color, glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
  // FIX: Set default radius to 1.0 as requested.
  m_Properties.Add(PropertyNames::Radius, 1.0f);
  RebuildMesh();
}

std::string Sphere::GetTypeString() const {
  return std::string(ObjectTypes::Sphere);
}

std::vector<GizmoHandleDef> Sphere::GetGizmoHandleDefs() {
  std::vector<GizmoHandleDef> defs;
  defs.push_back({PropertyNames::Scale, {1.0f, 0.0f, 0.0f}, {1, 0, 0, 1}});
  defs.push_back({PropertyNames::Scale, {0.0f, 1.0f, 0.0f}, {0, 1, 0, 1}});
  defs.push_back({PropertyNames::Scale, {0.0f, 0.0f, 1.0f}, {0, 0, 1, 1}});
  return defs;
}

void Sphere::OnGizmoUpdate(const std::string& propertyName, float delta,
                           const glm::vec3& axis) {
  if (propertyName == PropertyNames::Scale) {
    glm::vec3 currentScale =
        m_Properties.GetValue<glm::vec3>(PropertyNames::Scale);

    // FIX: Apply the delta only to the axis that was dragged
    glm::vec3 scaleChange = axis * delta;
    glm::vec3 newScale = currentScale + scaleChange;

    // Prevent scaling to zero or negative values on any axis
    newScale = glm::max(newScale, glm::vec3(0.05f));

    m_Properties.SetValue<glm::vec3>(PropertyNames::Scale, newScale);
  }
}

void Sphere::BuildMeshData(std::vector<float>& vertices,
                           std::vector<unsigned int>& indices) {
  vertices.clear();
  indices.clear();
  float radius = m_Properties.GetValue<float>(PropertyNames::Radius);
  int sectors = 36;
  int stacks = 18;
  float x, y, z, xy;
  float sectorStep = 2.0f * M_PI / sectors;
  float stackStep = M_PI / stacks;
  float sectorAngle, stackAngle;
  for (int i = 0; i <= stacks; ++i) {
    stackAngle = M_PI / 2 - i * stackStep;
    xy = radius * cosf(stackAngle);
    z = radius * sinf(stackAngle);
    for (int j = 0; j <= sectors; ++j) {
      sectorAngle = j * sectorStep;
      x = xy * cosf(sectorAngle);
      y = xy * sinf(sectorAngle);
      vertices.push_back(x);
      vertices.push_back(y);
      vertices.push_back(z);
    }
  }
  int k1, k2;
  for (int i = 0; i < stacks; ++i) {
    k1 = i * (sectors + 1);
    k2 = k1 + sectors + 1;
    for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
      if (i != 0) {
        indices.push_back(k1);
        indices.push_back(k2);
        indices.push_back(k1 + 1);
      }
      if (i != (stacks - 1)) {
        indices.push_back(k1 + 1);
        indices.push_back(k2);
        indices.push_back(k2 + 1);
      }
    }
  }
}