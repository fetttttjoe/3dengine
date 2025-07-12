#include "Factories/SceneObjectFactory.h"

#include "Core/Log.h"
#include "Interfaces.h"
#include "nlohmann/json.hpp"

void SceneObjectFactory::Register(const std::string& typeName, CreateFunc func) {
    // Create a temporary instance to query its properties
    auto tempInstance = func();
    bool isCreatable = tempInstance->IsUserCreatable();

    // Use emplace to construct the RegistryEntry directly in the map
    // This avoids the previous move/copy issues and is more efficient.
    m_Registry.emplace(typeName, RegistryEntry{ std::move(func), isCreatable });
}

std::unique_ptr<ISceneObject> SceneObjectFactory::Create(
    const std::string& typeName) const {
    auto it = m_Registry.find(typeName);
    if (it != m_Registry.end()) {
        // Correctly call the createFunc from the found entry
        return it->second.createFunc();
    }
    Log::Debug("[SceneObjectFactory] Error: Unknown object type '", typeName,
        "'");
    return nullptr;
}

std::unique_ptr<ISceneObject> SceneObjectFactory::Copy(
    const ISceneObject& src) const {
    auto clone = Create(src.GetTypeString());
    if (!clone) {
        Log::Debug(
            "[SceneObjectFactory::Copy] Failed to create a new instance of '",
            src.GetTypeString(), "' for copying.");
        return nullptr;
    }
    nlohmann::json j;
    src.Serialize(j);
    clone->Deserialize(j);
    return clone;
}

std::vector<std::string> SceneObjectFactory::GetUserCreatableTypeNames() const {
    std::vector<std::string> names;
    for (const auto& pair : m_Registry) {
        if (pair.second.isUserCreatable) {
            names.push_back(pair.first);
        }
    }
    return names;
}