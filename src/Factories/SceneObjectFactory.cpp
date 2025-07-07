#include "Factories/SceneObjectFactory.h"

#include <iostream>

#include "Interfaces.h"       // for ISceneObject
#include "nlohmann/json.hpp"  // for JSON round-trip

void SceneObjectFactory::Register(const std::string& typeName,
                                  CreateFunc func) {
  m_Registry[typeName] = std::move(func);
}

std::unique_ptr<ISceneObject> SceneObjectFactory::Create(
    const std::string& typeName) const {
  auto it = m_Registry.find(typeName);
  if (it != m_Registry.end()) return it->second();
  std::cerr << "[SceneObjectFactory] Unknown type: " << typeName << "\n";
  return nullptr;
}

std::unique_ptr<ISceneObject> SceneObjectFactory::Copy(
    const ISceneObject& src) const {
  // 1) blank clone
  auto clone = Create(src.GetTypeString());
  if (!clone) {
    std::cerr << "[SceneObjectFactory::Copy] failed to Create("
              << src.GetTypeString() << ")\n";
    return nullptr;
  }
  // 2) serialize → deserialize
  nlohmann::json j;
  src.Serialize(j);
  clone->Deserialize(j);
  return clone;
}

std::vector<std::string> SceneObjectFactory::GetRegisteredTypeNames() const {
  std::vector<std::string> names;
  names.reserve(m_Registry.size());
  for (auto const& kv : m_Registry) names.push_back(kv.first);
  return names;
}
