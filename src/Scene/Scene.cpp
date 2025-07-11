#include "Scene/Scene.h"

#include <algorithm>
#include <fstream>
#include <glm/glm.hpp>
#include <iomanip>
#include <iostream>

#include "Core/Log.h"
#include "Core/SettingsManager.h"
#include "Factories/SceneObjectFactory.h"
#include "Interfaces.h"
#include "Scene/Objects/CustomMesh.h"
#include "nlohmann/json.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

Scene::Scene(SceneObjectFactory* factory) : m_ObjectFactory(factory) {}

Scene::~Scene() = default;

void Scene::Clear() {
  // Only remove selectable objects. Non-selectable ones like the grid should
  // persist.
  m_Objects.erase(
      std::remove_if(m_Objects.begin(), m_Objects.end(),
                     [](const auto& obj) { return obj->isSelectable; }),
      m_Objects.end());

  m_DeferredDeletions.clear();
  m_SelectedIndex = -1;

  // Recalculate the next ID based on the objects that remain (e.g., the grid)
  uint32_t maxId = 0;
  for (const auto& o : m_Objects) {
    if (o) maxId = std::max(maxId, o->id);
  }
  m_NextObjectID = maxId + 1;
}
void Scene::ProcessDeferredDeletions() {
  if (m_DeferredDeletions.empty()) {
    return;
  }

  ISceneObject* selectedObject = GetSelectedObject();
  if (selectedObject != nullptr) {
    bool isSelectedForDeletion =
        std::find(m_DeferredDeletions.begin(), m_DeferredDeletions.end(),
                  selectedObject->id) != m_DeferredDeletions.end();
    if (isSelectedForDeletion) {
      SetSelectedObjectByID(0);
    }
  }

  m_Objects.erase(std::remove_if(m_Objects.begin(), m_Objects.end(),
                                 [this](const auto& obj) {
                                   return std::find(m_DeferredDeletions.begin(),
                                                    m_DeferredDeletions.end(),
                                                    obj->id) !=
                                          m_DeferredDeletions.end();
                                 }),
                  m_Objects.end());

  m_DeferredDeletions.clear();
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

std::pair<std::vector<float>, std::vector<unsigned int>>
Scene::LoadMeshFromFile(const std::string& filepath) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                        filepath.c_str())) {
    Log::Debug("Failed to load/parse .obj file: ", filepath);
    Log::Debug("Warn: ", warn);
    Log::Debug("Err: ", err);
    return {};
  }

  std::vector<float> vertices;
  std::vector<unsigned int> indices;
  std::unordered_map<glm::vec3, uint32_t, Vec3Hash> uniqueVertices{};

  for (const auto& shape : shapes) {
    for (const auto& index : shape.mesh.indices) {
      glm::vec3 vertex{};
      vertex.x = attrib.vertices[3 * index.vertex_index + 0];
      vertex.y = attrib.vertices[3 * index.vertex_index + 1];
      vertex.z = attrib.vertices[3 * index.vertex_index + 2];

      if (uniqueVertices.count(vertex) == 0) {
        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size() / 3);
        vertices.push_back(vertex.x);
        vertices.push_back(vertex.y);
        vertices.push_back(vertex.z);
      }
      indices.push_back(uniqueVertices[vertex]);
    }
  }
  return {vertices, indices};
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

void Scene::QueueForDeletion(uint32_t id) {
  if (std::find(m_DeferredDeletions.begin(), m_DeferredDeletions.end(), id) ==
      m_DeferredDeletions.end()) {
    m_DeferredDeletions.push_back(id);
  }
}

void Scene::DeleteSelectedObject() {
  if (auto* s = GetSelectedObject()) {
    QueueForDeletion(s->id);
  }
}

ISceneObject* Scene::GetSelectedObject() {
  if (m_SelectedIndex >= 0 && m_SelectedIndex < (int)m_Objects.size()) {
    return m_Objects[m_SelectedIndex].get();
  }
  return nullptr;
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
    clone->SetPosition(origPos + SettingsManager::Get().cloneOffset);
  }

  m_Objects.push_back(std::move(clone));
}