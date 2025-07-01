// =======================================================================
// File: src/Scene/Scene.cpp
// =======================================================================
#include "Scene/Scene.h"
#include "Interfaces.h"
#include "Scene/Grid.h" // Required for dynamic_cast<Grid*>
#include "Factories/SceneObjectFactory.h" // FIX: Include factory for DuplicateObject
#include <algorithm> // For std::remove_if
#include <iostream> // For debug output

// FIX: Update constructor to accept SceneObjectFactory*
Scene::Scene(SceneObjectFactory* factory) : m_ObjectFactory(factory) {}
Scene::~Scene() {}

void Scene::AddObject(std::unique_ptr<ISceneObject> object) {
    if (object) {
        object->id = m_NextObjectID++;
        m_Objects.push_back(std::move(object));
    }
}

const std::vector<std::unique_ptr<ISceneObject>>& Scene::GetSceneObjects() const {
    return m_Objects;
}

ISceneObject* Scene::GetObjectByID(uint32_t id) {
    for (const auto& obj : m_Objects) {
        if (obj->id == id) {
            return obj.get();
        }
    }
    return nullptr;
}

void Scene::SetSelectedObjectByID(uint32_t id) {
    if (m_SelectedIndex != -1 && m_SelectedIndex < m_Objects.size()) {
        m_Objects[m_SelectedIndex]->isSelected = false;
    }

    m_SelectedIndex = -1;

    for (int i = 0; i < m_Objects.size(); ++i) {
        if (m_Objects[i]->id == id) {
            m_SelectedIndex = i;
            m_Objects[i]->isSelected = true;
            break;
        }
    }
    if (id == 0) {
         m_SelectedIndex = -1;
    }
}


void Scene::SelectNextObject() {
    int startIndex = (m_SelectedIndex == -1) ? 0 : m_SelectedIndex + 1;

    for (int i = 0; i < m_Objects.size(); ++i) {
        int currentIndex = (startIndex + i) % m_Objects.size();
        if (!dynamic_cast<Grid*>(m_Objects[currentIndex].get())) {
            SetSelectedObjectByID(m_Objects[currentIndex]->id);
            return;
        }
    }
    SetSelectedObjectByID(0);
}


void Scene::DeleteSelectedObject() {
    if (m_SelectedIndex >= 0 && m_SelectedIndex < m_Objects.size()) {
        if (!dynamic_cast<Grid*>(m_Objects[m_SelectedIndex].get())) {
            m_Objects.erase(m_Objects.begin() + m_SelectedIndex);
            m_SelectedIndex = -1;
        } else {
            std::cout << "Attempted to delete Grid object via DeleteSelectedObject. Operation denied." << std::endl;
        }
    }
}

void Scene::DeleteObjectByID(uint32_t id) {
    auto it = std::remove_if(m_Objects.begin(), m_Objects.end(),
                             [id, this](const std::unique_ptr<ISceneObject>& obj) {
                                 if (obj->id == id && !dynamic_cast<Grid*>(obj.get())) {
                                     if (obj->isSelected) {
                                         m_SelectedIndex = -1;
                                     }
                                     return true;
                                 }
                                 return false;
                             });
    m_Objects.erase(it, m_Objects.end());
}


void Scene::DuplicateObject(uint32_t id) {
    ISceneObject* original = GetObjectByID(id);
    if (original) {
        // FIX: Check if factory is set before use
        if (!m_ObjectFactory) {
            std::cerr << "Error: Scene object factory not set, cannot duplicate object." << std::endl;
            return;
        }

        std::unique_ptr<ISceneObject> newObject = m_ObjectFactory->Create(original->GetTypeString());
        if (newObject) {
            newObject->name = original->name + " (Copy)";
            newObject->transform = original->transform * glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 0.0f));
            AddObject(std::move(newObject));
        } else {
            std::cerr << "Failed to duplicate object with ID: " << id << ". Type: " << original->GetTypeString() << std::endl;
        }
    } else {
        std::cerr << "Attempted to duplicate non-existent object with ID: " << id << std::endl;
    }
}

ISceneObject* Scene::GetSelectedObject() {
    if (m_SelectedIndex >= 0 && m_SelectedIndex < m_Objects.size()) {
        return m_Objects[m_SelectedIndex].get();
    }
    return nullptr;
}