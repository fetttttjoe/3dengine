#pragma once

#include <cstdint>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "Core/Log.h"
#include "Core/PropertyNames.h"
#include "imgui.h"
#include "imgui_stdlib.h"

class Scene;
class Shader;
class IProperty;
class SculptableMesh;

class PropertySet {
 public:
  template <typename T>
  void Add(const std::string& name, T initialValue,
           std::function<void()> onChangeCallback = nullptr);
  template <typename T>
  T GetValue(const std::string& name) const;
  template <typename T>
  void SetValue(const std::string& name, const T& value);
  IProperty* GetProperty(const std::string& name) const;
  const std::vector<std::unique_ptr<IProperty>>& GetProperties() const;
  void Serialize(nlohmann::json& outJson) const;
  void Deserialize(const nlohmann::json& inJson);

 private:
  std::unordered_map<std::string, IProperty*> m_PropertyMap;
  std::vector<std::unique_ptr<IProperty>> m_Properties;
};

class IProperty {
 public:
  virtual ~IProperty() = default;
  virtual const std::string& GetName() const = 0;
  virtual void DrawEditor() = 0;
  virtual void Serialize(nlohmann::json& j) = 0;
  virtual void Deserialize(const nlohmann::json& j) = 0;
  void SetChangeCallback(std::function<void()> callback);

 protected:
  std::string m_Name;
  std::function<void()> m_OnChangeCallback;
};

template <typename T>
class TProperty : public IProperty {
 public:
  TProperty(std::string name, T value);
  const std::string& GetName() const override;
  T GetValue() const;
  void SetValue(const T& value);
  void Serialize(nlohmann::json& outJson) override;
  void Deserialize(const nlohmann::json& inJson) override;
  void DrawEditor() override;

 private:
  T m_Value;
};

struct GizmoHandleDef {
  std::string propertyName;
  glm::vec3 localDirection;
  glm::vec4 color;
};

class IGizmoClient {
 public:
  virtual ~IGizmoClient() = default;
  virtual std::vector<GizmoHandleDef> GetGizmoHandleDefs() = 0;
  virtual void OnGizmoUpdate(const std::string& propertyName, float delta,
                             const glm::vec3& axis) = 0;
};

class ISceneObject : public IGizmoClient {
 public:
  ISceneObject() : id(0), name("Unnamed Object"), isSelected(false) {}
  virtual ~ISceneObject() = default;

  virtual void Draw(class OpenGLRenderer& renderer, const glm::mat4& view,
                    const glm::mat4& projection) = 0;
  virtual void DrawForPicking(Shader& pickingShader, const glm::mat4& view,
                              const glm::mat4& projection) = 0;
  virtual void DrawHighlight(const glm::mat4& view,
                             const glm::mat4& projection) const = 0;
  virtual std::string GetTypeString() const = 0;
  virtual PropertySet& GetPropertySet() = 0;
  virtual const PropertySet& GetPropertySet() const = 0;
  virtual void RebuildMesh() = 0;
  virtual const glm::mat4& GetTransform() const = 0;
  virtual glm::vec3 GetPosition() const = 0;
  virtual glm::quat GetRotation() const = 0;
  virtual glm::vec3 GetScale() const = 0;
  virtual void SetPosition(const glm::vec3& position) = 0;
  virtual void SetRotation(const glm::quat& rotation) = 0;
  virtual void SetScale(const glm::vec3& scale) = 0;
  virtual void SetEulerAngles(const glm::vec3& eulerAngles) = 0;
  virtual void Serialize(nlohmann::json& outJson) const;
  virtual void Deserialize(const nlohmann::json& inJson);

  virtual SculptableMesh* GetSculptableMesh() = 0;
  virtual bool IsMeshDirty() const = 0;
  virtual void SetMeshDirty(bool dirty) = 0;
  virtual bool IsUserCreatable() const { return true; }

  uint32_t id;
  std::string name;
  bool isSelected;
  bool isSelectable = true;
  bool isStatic = false;
  bool isPristine = true;
};

// =======================================================================
// Template Implementations
// =======================================================================

template <typename T>
void PropertySet::Add(const std::string& name, T initialValue,
                      std::function<void()> onChangeCallback) {
  auto prop = std::make_unique<TProperty<T>>(name, initialValue);
  if (onChangeCallback) {
    prop->SetChangeCallback(onChangeCallback);
  }
  m_PropertyMap[name] = prop.get();
  m_Properties.push_back(std::move(prop));
}

template <typename T>
T PropertySet::GetValue(const std::string& name) const {
  auto it = m_PropertyMap.find(name);
  if (it == m_PropertyMap.end()) {
    throw std::runtime_error("Property not found: " + name);
  }
  auto castedProp = dynamic_cast<TProperty<T>*>(it->second);
  if (!castedProp) {
    throw std::runtime_error("Invalid type for property: " + name);
  }
  return castedProp->GetValue();
}

template <typename T>
void PropertySet::SetValue(const std::string& name, const T& value) {
  auto it = m_PropertyMap.find(name);
  if (it == m_PropertyMap.end()) {
    throw std::runtime_error("Property not found: " + name);
  }
  auto castedProp = dynamic_cast<TProperty<T>*>(it->second);
  if (!castedProp) {
    throw std::runtime_error("Invalid type for property: " + name);
  }
  castedProp->SetValue(value);
}

inline IProperty* PropertySet::GetProperty(const std::string& name) const {
  auto it = m_PropertyMap.find(name);
  return (it != m_PropertyMap.end()) ? it->second : nullptr;
}

inline const std::vector<std::unique_ptr<IProperty>>&
PropertySet::GetProperties() const {
  return m_Properties;
}

inline void PropertySet::Serialize(nlohmann::json& outJson) const {
  for (const auto& prop : m_Properties) {
    prop->Serialize(outJson);
  }
}

inline void PropertySet::Deserialize(const nlohmann::json& inJson) {
  for (auto& prop : m_Properties) {
    prop->Deserialize(inJson);
  }
}

inline void IProperty::SetChangeCallback(std::function<void()> callback) {
  m_OnChangeCallback = callback;
}

template <typename T>
TProperty<T>::TProperty(std::string name, T value) : m_Value(value) {
  m_Name = name;
}

template <typename T>
const std::string& TProperty<T>::GetName() const {
  return m_Name;
}

template <typename T>
T TProperty<T>::GetValue() const {
  return m_Value;
}

template <typename T>
void TProperty<T>::SetValue(const T& value) {
  m_Value = value;
  if (m_OnChangeCallback) {
    m_OnChangeCallback();
  }
}

template <typename T>
void TProperty<T>::Serialize(nlohmann::json& outJson) {
  outJson[m_Name] = m_Value;
}

template <typename T>
void TProperty<T>::Deserialize(const nlohmann::json& inJson) {
  if (inJson.contains(m_Name)) {
    inJson.at(m_Name).get_to(m_Value);
  }
}

template <typename T>
void TProperty<T>::DrawEditor() {
  T tempValue = m_Value;
  bool changed = false;
  if constexpr (std::is_same_v<T, float>) {
    changed = ImGui::DragFloat(m_Name.c_str(), &tempValue, 0.05f);
  } else if constexpr (std::is_same_v<T, glm::vec3>) {
    changed =
        ImGui::DragFloat3(m_Name.c_str(), glm::value_ptr(tempValue), 0.1f);
  } else if constexpr (std::is_same_v<T, glm::vec4>) {
    if (m_Name == PropertyNames::Color) {
      changed = ImGui::ColorEdit4(m_Name.c_str(), glm::value_ptr(tempValue));
    } else {
      changed =
          ImGui::DragFloat4(m_Name.c_str(), glm::value_ptr(tempValue), 0.05f);
    }
  } else {
    ImGui::Text("%s: (Unsupported Type)", m_Name.c_str());
  }
  if (changed) {
    SetValue(tempValue);
  }
}

namespace glm {
void to_json(nlohmann::json& j, const glm::vec3& v);
void from_json(const nlohmann::json& j, glm::vec3& v);
void to_json(nlohmann::json& j, const glm::vec4& v);
void from_json(const nlohmann::json& j, glm::vec4& v);
void to_json(nlohmann::json& j, const glm::quat& q);
void from_json(const nlohmann::json& j, glm::quat& q);
}  // namespace glm