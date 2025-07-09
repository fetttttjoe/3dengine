#include "Scene.h"

#include <Core/SettingsManager.h>

#include <algorithm>
#include <fstream>
#include <glm/glm.hpp>
#include <iomanip>
#include <iostream>

#include "Factories/SceneObjectFactory.h"
#include "Interfaces.h"
#include "nlohmann/json.hpp"

Scene::Scene(SceneObjectFactory* factory) : m_ObjectFactory(factory) {}

Scene::~Scene() = default;

void Scene::Clear() {
  m_Objects.erase(
      std::remove_if(m_Objects.begin(), m_Objects.end(),
                     [](auto const& obj) { return obj->isSelectable; }),
      m_Objects.end());
  m_SelectedIndex = -1;
  m_NextObjectID = 1;
  for (auto const& o : m_Objects)
    m_NextObjectID = std::max(m_NextObjectID, o->id + 1);
}

void Scene::Save(const std::string& filepath) const {
  nlohmann::json sceneJ;
  sceneJ["objects"] = nlohmann::json::array();

  uint32_t maxId = 0;
  for (auto const& o : m_Objects) {
    if (o->isSelectable) {
      nlohmann::json oj;
      o->Serialize(oj);
      sceneJ["objects"].push_back(oj);
    }
    maxId = std::max(maxId, o->id);
  }
  sceneJ["next_object_id"] = maxId + 1;

  std::ofstream ofs(filepath);
  ofs << std::setw(4) << sceneJ << "\n";
}

void Scene::Load(const std::string& filepath) {
  std::ifstream in(filepath);
  if (!in.is_open()) {
    Log::Debug("Could not open scene file for loading: ", filepath);
    return;
  }

  nlohmann::json sceneJson;
  in >> sceneJson;

  Clear();

  m_NextObjectID = sceneJson.value("next_object_id", 1);

  const auto& arr = sceneJson["objects"];
  for (const auto& objJson : arr) {
    std::string type = objJson.value("type", "");
    auto clone = m_ObjectFactory->Create(type);
    if (!clone) continue;

    clone->Deserialize(objJson);

    if (clone->id >= m_NextObjectID) m_NextObjectID = clone->id + 1;

    m_Objects.push_back(std::move(clone));
  }
}

void Scene::AddObject(std::unique_ptr<ISceneObject> object) {
  if (!object) return;
  object->id = m_NextObjectID++;
  m_Objects.push_back(std::move(object));
}

const std::vector<std::unique_ptr<ISceneObject>>& Scene::GetSceneObjects()
    const {
  return m_Objects;
}

// <<< MODIFIED: Added function definitions back to the .cpp file
ISceneObject* Scene::GetObjectByID(uint32_t id) {
  auto it = std::find_if(m_Objects.begin(), m_Objects.end(),
                         [id](auto const& o) { return o->id == id; });
  return (it != m_Objects.end() ? it->get() : nullptr);
}

const ISceneObject* Scene::GetObjectByID(uint32_t id) const {
  auto it = std::find_if(m_Objects.begin(), m_Objects.end(),
                         [id](auto const& o) { return o->id == id; });
  return (it != m_Objects.end() ? it->get() : nullptr);
}

void Scene::SetSelectedObjectByID(uint32_t id) {
  if (m_SelectedIndex >= 0 && m_SelectedIndex < (int)m_Objects.size())
    m_Objects[m_SelectedIndex]->isSelected = false;

  m_SelectedIndex = -1;
  for (int i = 0; i < (int)m_Objects.size(); ++i) {
    if (m_Objects[i]->id == id && m_Objects[i]->isSelectable) {
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
  int start = (m_SelectedIndex < 0 ? 0 : m_SelectedIndex + 1);
  for (int d = 0; d < (int)m_Objects.size(); ++d) {
    int i = (start + d) % m_Objects.size();
    if (m_Objects[i]->isSelectable) {
      SetSelectedObjectByID(m_Objects[i]->id);
      return;
    }
  }
  SetSelectedObjectByID(0);
}

void Scene::DeleteObjectByID(uint32_t id) {
  auto it = std::find_if(m_Objects.begin(), m_Objects.end(),
                         [id](auto const& o) { return o->id == id; });
  if (it == m_Objects.end() || !(*it)->isSelectable) return;

  bool wasSelected = ((*it)->isSelected);
  m_Objects.erase(it);
  if (wasSelected) m_SelectedIndex = -1;
}

void Scene::DeleteSelectedObject() {
  if (auto* s = GetSelectedObject()) DeleteObjectByID(s->id);
}

ISceneObject* Scene::GetSelectedObject() {
  return (m_SelectedIndex >= 0 && m_SelectedIndex < (int)m_Objects.size())
             ? m_Objects[m_SelectedIndex].get()
             : nullptr;
}

int Scene::GetNextAvailableIndexForName(const std::string& baseName) const {
  int maxNum = 0;
  bool foundBase = false;
  for (auto const& o : m_Objects) {
    if (o->name == baseName) foundBase = true;
    if (o->name.rfind(baseName + " (", 0) == 0) {
      try {
        auto num = std::stoi(o->name.substr(
            baseName.size() + 2, o->name.size() - baseName.size() - 3));
        maxNum = std::max(maxNum, num);
      } catch (...) {
      }
    }
  }
  if (foundBase && maxNum == 0) return 1;
  return maxNum + 1;
}

void Scene::DuplicateObject(uint32_t sourceID) {
  ISceneObject* orig = GetObjectByID(sourceID);
  if (!orig || !orig->isSelectable || !m_ObjectFactory) return;

  auto clone = m_ObjectFactory->Copy(*orig);
  if (!clone) return;

  clone->id = m_NextObjectID++;

  {
    std::string base = orig->GetTypeString();
    int idx = GetNextAvailableIndexForName(base);
    clone->name = (idx == 0) ? base : base + " (" + std::to_string(idx) + ")";
  }

  {
    auto p = orig->GetPosition();
    glm::vec3 origPos = orig->GetPosition();
    glm::vec3 offset = SettingsManager::Get().cloneOffset;
    clone->SetPosition(origPos + offset);
  }

  m_Objects.push_back(std::move(clone));
}
