#include "Scene.h"
#include "Interfaces.h"
#include "Grid.h"
#include "Factories/SceneObjectFactory.h"
#include <algorithm>
#include <iostream>
#include <string>

Scene::Scene(SceneObjectFactory* factory) : m_ObjectFactory(factory) {}
Scene::~Scene() {}

// Private helper to get the next available number for a given name.
int Scene::GetNextAvailableIndexForName(const std::string& baseName) {
    int maxNumber = 0;
    
    // Check the base name itself. If "Pyramid" exists, the first copy should be "Pyramid (1)".
    for (const auto& obj : m_Objects) {
        if (obj->name == baseName) {
            maxNumber = std::max(maxNumber, 0);
        }
        // Check for numbered copies like "Pyramid (1)"
        else if (obj->name.rfind(baseName + " (", 0) == 0) {
            try {
                size_t start = baseName.length() + 2; // Move past "<baseName> ("
                size_t end = obj->name.length() - 1;   // Move before ")"
                int num = std::stoi(obj->name.substr(start, end - start));
                if (num > maxNumber) {
                    maxNumber = num;
                }
            } catch (const std::exception&) {
                // Ignore if parsing fails, it's not a valid numbered copy.
            }
        }
    }
    return maxNumber + 1;
}

void Scene::AddObject(std::unique_ptr<ISceneObject> object) {
    if (object) {
        object->id = m_NextObjectID++;
        m_Objects.push_back(std::move(object));
    }
}

const std::vector<std::unique_ptr<ISceneObject>>& Scene::GetSceneObjects() const {
    return m_Objects;
}

// REFACTORED: Replaced manual for-loop with std::find_if for clarity.
ISceneObject* Scene::GetObjectByID(uint32_t id) {
    auto it = std::find_if(m_Objects.begin(), m_Objects.end(),
        [id](const std::unique_ptr<ISceneObject>& obj) {
            return obj->id == id;
        });

    if (it != m_Objects.end()) {
        return it->get();
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
}

void Scene::SelectNextObject() {
    if (m_Objects.empty()) {
        SetSelectedObjectByID(0);
        return;
    }

    int startIndex = (m_SelectedIndex == -1) ? 0 : m_SelectedIndex + 1;

    for (size_t i = 0; i < m_Objects.size(); ++i) {
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

// REFACTORED: Safer deletion logic. First find, then update state, then erase.
void Scene::DeleteObjectByID(uint32_t id) {
    auto it = std::find_if(m_Objects.begin(), m_Objects.end(),
        [id](const std::unique_ptr<ISceneObject>& obj) {
            return obj->id == id;
        });

    if (it != m_Objects.end()) {
        // Don't delete the grid.
        if (dynamic_cast<Grid*>(it->get())) {
            return;
        }

        // If the object being deleted is the currently selected one, deselect it.
        if (it->get() == GetSelectedObject()) {
            m_SelectedIndex = -1;
        }
        
        m_Objects.erase(it);
    }
}

// REFACTORED: Implemented smart naming for duplicates.
void Scene::DuplicateObject(uint32_t id) {
    ISceneObject* original = GetObjectByID(id);
    if (!original) {
        std::cerr << "Attempted to duplicate non-existent object with ID: " << id << std::endl;
        return;
    }

    if (!m_ObjectFactory) {
        std::cerr << "Error: Scene object factory not set, cannot duplicate object." << std::endl;
        return;
    }

    std::unique_ptr<ISceneObject> newObject = m_ObjectFactory->Create(original->GetTypeString());
    if (newObject) {
        // Smart Naming Logic
        std::string baseName = original->name;
        size_t pos = baseName.rfind(" (");
        if (pos != std::string::npos) {
            baseName = baseName.substr(0, pos);
        }
        int nextIndex = GetNextAvailableIndexForName(baseName);
        newObject->name = baseName + " (" + std::to_string(nextIndex) + ")";
        
        // Copy transform and slightly offset
        newObject->transform = original->transform * glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 0.0f));
        AddObject(std::move(newObject));
    } else {
        std::cerr << "Failed to duplicate object with ID: " << id << ". Type: " << original->GetTypeString() << std::endl;
    }
}

ISceneObject* Scene::GetSelectedObject() {
    if (m_SelectedIndex >= 0 && m_SelectedIndex < m_Objects.size()) {
        return m_Objects[m_SelectedIndex].get();
    }
    return nullptr;
}