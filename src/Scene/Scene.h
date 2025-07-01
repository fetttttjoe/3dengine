// =======================================================================
// File: src/Scene/Scene.h
// =======================================================================
#pragma once
#include <vector>
#include <memory>
#include <cstdint> // For uint32_t

class ISceneObject; // Forward declaration
class SceneObjectFactory; // FIX: Forward declare SceneObjectFactory

class Scene {
public:
    // FIX: Add SceneObjectFactory* parameter to constructor
    Scene(SceneObjectFactory* factory = nullptr);
    ~Scene();

    void AddObject(std::unique_ptr<ISceneObject> object);
    const std::vector<std::unique_ptr<ISceneObject>>& GetSceneObjects() const;

    ISceneObject* GetObjectByID(uint32_t id);

    void SelectNextObject();
    void SetSelectedObjectByID(uint32_t id);
    void DeleteSelectedObject();
    void DeleteObjectByID(uint32_t id);
    void DuplicateObject(uint32_t id); // This method uses the factory

    ISceneObject* GetSelectedObject();

private:
    std::vector<std::unique_ptr<ISceneObject>> m_Objects;
    int m_SelectedIndex = -1;
    uint32_t m_NextObjectID = 1;

    // FIX: Add member to store the SceneObjectFactory
    SceneObjectFactory* m_ObjectFactory = nullptr;
};