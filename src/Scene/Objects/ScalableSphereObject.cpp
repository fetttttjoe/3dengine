#include "Scene/Objects/ScalableSphereObject.h"

#include "Core/PropertyNames.h"

void ScalableSphereObject::OnGizmoUpdate(const std::string& propertyName,
                                         float delta, const glm::vec3& axis) {
  if (propertyName == PropertyNames::Scale) {
    glm::vec3 currentScale =
        m_Properties.GetValue<glm::vec3>(PropertyNames::Scale);
    glm::vec3 scaleChange(std::abs(axis.x) > 0.5f ? delta : 0.0f,
                          std::abs(axis.y) > 0.5f ? delta : 0.0f,
                          std::abs(axis.z) > 0.5f ? delta : 0.0f);
    glm::vec3 newScale = currentScale + scaleChange;
    newScale = glm::max(newScale, glm::vec3(0.05f));
    m_Properties.SetValue<glm::vec3>(PropertyNames::Scale, newScale);
  } else {
    BaseObject::OnGizmoUpdate(propertyName, delta, axis);
  }
}