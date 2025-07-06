#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class ISceneObject;

/**
 * @brief Simple factory for creating and cloning scene objects.
 */
class SceneObjectFactory {
 public:
  using CreateFunc = std::function<std::unique_ptr<ISceneObject>()>;

  /// Register a creator function for the given type name.
  void Register(const std::string& typeName, CreateFunc func);

  /// Create a brand-new instance by type name.
  std::unique_ptr<ISceneObject> Create(const std::string& typeName) const;

  /// Deep-copy @p src into a new ISceneObject of the same dynamic type.
  std::unique_ptr<ISceneObject> Copy(const ISceneObject& src) const;

  /// List of all registered type names.
  std::vector<std::string> GetRegisteredTypeNames() const;

 private:
  std::unordered_map<std::string, CreateFunc> m_Registry;
};
