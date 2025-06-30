
// =======================================================================
// File: src/Scene/Scene.cpp
// =======================================================================
#include "Scene/Scene.h"
#include "Interfaces.h"

Scene::Scene() {}
Scene::~Scene() {}

void Scene::AddObject(std::unique_ptr<ISceneObject> object) {
    m_Objects.push_back(std::move(object));
}
const std::vector<std::unique_ptr<ISceneObject>>& Scene::GetSceneObjects() const {
    return m_Objects;
}

void Scene::SelectNextObject() {
    if (m_Objects.size() > 1) { // More than just the grid
        m_SelectedIndex++;
        if (m_SelectedIndex >= m_Objects.size() || m_SelectedIndex == 0) { // Skip grid
            m_SelectedIndex = 1;
        }
    }
}

ISceneObject* Scene::GetSelectedObject() {
    if (m_SelectedIndex >= 0 && m_SelectedIndex < m_Objects.size()) {
        return m_Objects[m_SelectedIndex].get();
    }
    return nullptr;
}
