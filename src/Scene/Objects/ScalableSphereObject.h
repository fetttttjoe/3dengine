#pragma once

#include "Scene/Objects/BaseObject.h"

// This helper class contains the shared scaling logic for all spherical
// objects.
class ScalableSphereObject : public BaseObject {
 public:
  void OnGizmoUpdate(const std::string& propertyName, float delta,
                     const glm::vec3& axis) override;
};