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
    int maxNum = 0;
    // Iterate through all objects to find the highest existing number for a given base name
    for (const auto& obj : m_Objects) {
        // Check for numbered copies like "Pyramid (1)"
        if (obj->name.rfind(baseName + " (", 0) == 0) {
            try {
                size_t start = baseName.length() + 2; // Move past "<baseName> ("
                size_t end = obj->name.length() - 1;   // Move before ")"
                int num = std::stoi(obj->name.substr(start, end - start));
                if (num > maxNum) {
                    maxNum = num;
                }
            } catch (const std::exception&) {
                // Ignore if parsing fails, it's not a valid numbered copy.
            }
        }
    }
    return maxNum + 1;
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
            if (m_Objects[i]->isSelectable) {
                m_SelectedIndex = i;
                m_Objects[i]->isSelected = true;
            }
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
        if (m_Objects[currentIndex]->isSelectable) {
            SetSelectedObjectByID(m_Objects[currentIndex]->id);
            return;
        }
    }
    SetSelectedObjectByID(0);
}

void Scene::DeleteSelectedObject() {
    if (m_SelectedIndex >= 0 && m_SelectedIndex < m_Objects.size()) {
        if (m_Objects[m_SelectedIndex]->isSelectable) {
            m_Objects.erase(m_Objects.begin() + m_SelectedIndex);
            m_SelectedIndex = -1;
        } else {
            std::cout << "Attempted to delete a non-selectable object. Operation denied." << std::endl;
        }
    }
}
// In file: src/Scene/Scene.cpp

void Scene::DeleteObjectByID(uint32_t id) {
    auto it = std::find_if(m_Objects.begin(), m_Objects.end(),
        [id](const std::unique_ptr<ISceneObject>& obj) {
            return obj->id == id;
        });

    if (it != m_Objects.end()) {
        if (!it->get()->isSelectable) {
            return;
        }

        // Get the index of the object we are about to delete
        int deletedIndex = std::distance(m_Objects.begin(), it);

        if (it->get() == GetSelectedObject()) {
            // The object being deleted is the selected one, so reset selection.
            m_SelectedIndex = -1;
        }
        
        m_Objects.erase(it);

        if (m_SelectedIndex != -1 && deletedIndex < m_SelectedIndex) {
            m_SelectedIndex--;
        }
    }
}

void Scene::DuplicateObject(uint32_t id) {
    ISceneObject* original = GetObjectByID(id);
    if (!original) {
        std::cerr << "Attempted to duplicate non-existent object with ID: " << id << std::endl;
        return;
    }
    
    if (!original->isSelectable) {
        std::cerr << "Attempted to duplicate non-selectable object. Operation denied." << std::endl;
        return;
    }

    if (!m_ObjectFactory) {
        std::cerr << "Error: Scene object factory not set, cannot duplicate object." << std::endl;
        return;
    }

    std::unique_ptr<ISceneObject> newObject = m_ObjectFactory->Create(original->GetTypeString());
    if (newObject) {
        // FIX: The base name for a duplicate should come from its fundamental type,
        // not its current (potentially user-edited) name. This makes the logic
        // more robust and predictable.
        std::string baseName = original->GetTypeString();
        
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
