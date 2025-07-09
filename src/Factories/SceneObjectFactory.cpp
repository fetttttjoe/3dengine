#include "Factories/SceneObjectFactory.h"

#include "Core/Log.h"
#include "Interfaces.h"
#include "nlohmann/json.hpp"

void SceneObjectFactory::Register(const std::string& typeName,
                                  CreateFunc func) {
  m_Registry[typeName] = std::move(func);
}

std::unique_ptr<ISceneObject> SceneObjectFactory::Create(
    const std::string& typeName) const {
  auto it = m_Registry.find(typeName);
  if (it != m_Registry.end()) {
    return it->second();
  }
  Log::Debug("[SceneObjectFactory] Error: Unknown object type '", typeName,
             "'");
  return nullptr;
}

std::unique_ptr<ISceneObject> SceneObjectFactory::Copy(
    const ISceneObject& src) const {
  // 1) Create a new blank object of the same type
  auto clone = Create(src.GetTypeString());
  if (!clone) {
    Log::Debug(
        "[SceneObjectFactory::Copy] Failed to create a new instance of '",
        src.GetTypeString(), "' for copying.");
    return nullptr;
  }
  // 2) Use serialization to transfer the state
  nlohmann::json j;
  src.Serialize(j);
  clone->Deserialize(j);
  return clone;
}

std::vector<std::string> SceneObjectFactory::GetRegisteredTypeNames() const {
  std::vector<std::string> names;
  names.reserve(m_Registry.size());
  for (auto const& kv : m_Registry) {
    names.push_back(kv.first);
  }
  return names;
}