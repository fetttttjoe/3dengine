#pragma once

#include "Scene/Objects/BaseObject.h"

// This new class serves as a "helper" or intermediate base class.
// It contains the shared scaling logic for all spherical objects.
class ScalableSphereObject : public BaseObject {
 public:
  // It provides the specific OnGizmoUpdate implementation that Sphere and
  // Icosphere need.
  void OnGizmoUpdate(const std::string& propertyName, float delta,
                     const glm::vec3& axis) override;
};