#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class ISceneObject;

class SceneObjectFactory {
 public:
  using CreateFunc = std::function<std::unique_ptr<ISceneObject>()>;

  void Register(const std::string& typeName, CreateFunc func);

  std::unique_ptr<ISceneObject> Create(const std::string& typeName) const;

  std::unique_ptr<ISceneObject> Copy(const ISceneObject& src) const;

  std::vector<std::string> GetUserCreatableTypeNames() const;

 private:
  // This struct holds all the necessary info for each registered type
  struct RegistryEntry {
    CreateFunc createFunc;
    bool isUserCreatable;
  };

  // The map correctly stores the RegistryEntry struct
  std::unordered_map<std::string, RegistryEntry> m_Registry;
};