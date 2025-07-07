#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class ISceneObject;
class SceneObjectFactory;

/**
 * @brief Manages a collection of ISceneObject, selection, save/load,
 * duplication, etc.
 */
class Scene {
 public:
  explicit Scene(SceneObjectFactory* factory);
  ~Scene();

  /// Remove all selectable objects, reset IDs & selection.
  void Clear();

  /// Serialize only selectable objects to disk.
  void Save(const std::string& filepath) const;

  /// Load scene from disk, replacing existing objects.
  void Load(const std::string& filepath);

  /// Add a freshly constructed object (assigns it a new ID).
  void AddObject(std::unique_ptr<ISceneObject> object);

  /// Get full list (for UI outliner).
  const std::vector<std::unique_ptr<ISceneObject>>& GetSceneObjects() const;

  /// Duplicate an object by ID, carrying over all properties.
  void DuplicateObject(uint32_t sourceID);

  /// Remove by ID or remove current selection.
  void DeleteObjectByID(uint32_t id);
  void DeleteSelectedObject();

  /// Mark the object with @p id as selected (for gizmo & inspector).
  void SetSelectedObjectByID(uint32_t id);

  /// Cycle to next selectable.
  void SelectNextObject();

  /// Get raw pointer to currently selected (or nullptr).
  ISceneObject* GetSelectedObject();

  /// Find pointer by ID (or nullptr).
  ISceneObject* GetObjectByID(uint32_t id);

 private:
  /// Helper for naming duplicates: returns 0 if no conflict, else next integer.
  int GetNextAvailableIndexForName(const std::string& baseName) const;

  std::vector<std::unique_ptr<ISceneObject>> m_Objects;
  int m_SelectedIndex = -1;
  uint32_t m_NextObjectID = 1;
  SceneObjectFactory* m_ObjectFactory;
};
