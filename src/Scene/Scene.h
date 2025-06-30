
// =======================================================================
// File: src/Scene/Scene.h
// Description: Updated to manage a selected object.
// =======================================================================
#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>

class ISceneObject;

class Scene {
public:
    Scene();
    ~Scene();

    void AddObject(std::unique_ptr<ISceneObject> object);
    const std::vector<std::unique_ptr<ISceneObject>>& GetSceneObjects() const;
    
    // Selection management
    void SelectNextObject();
    ISceneObject* GetSelectedObject();

private:
    std::vector<std::unique_ptr<ISceneObject>> m_Objects;
    int m_SelectedIndex = -1;
};