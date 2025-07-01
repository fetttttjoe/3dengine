#pragma once
#include <vector>
#include <memory>
#include <string>
#include <cstdint>

// Forward declarations
class ISceneObject;
class SceneObjectFactory;

class Scene {
public:
    Scene(SceneObjectFactory* factory);
    ~Scene();

    void AddObject(std::unique_ptr<ISceneObject> object);
    const std::vector<std::unique_ptr<ISceneObject>>& GetSceneObjects() const;

    ISceneObject* GetObjectByID(uint32_t id);
    ISceneObject* GetSelectedObject();

    void SetSelectedObjectByID(uint32_t id);
    void SelectNextObject();
    void DeleteSelectedObject();
    void DeleteObjectByID(uint32_t id);
    void DuplicateObject(uint32_t id);

private:
    // Helper function to find the next available number for a duplicated object's name.
    int GetNextAvailableIndexForName(const std::string& baseName);

    std::vector<std::unique_ptr<ISceneObject>> m_Objects;
    int m_SelectedIndex = -1;
    uint32_t m_NextObjectID = 1;
    SceneObjectFactory* m_ObjectFactory = nullptr;
};