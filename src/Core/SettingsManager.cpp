#include "Core/SettingsManager.h"

#include <fstream>
#include <iomanip>

#include "Core/JsonGlmHelpers.h"
#include "Core/Log.h"
#include "nlohmann/json.hpp"

AppSettings SettingsManager::s_Settings;
SettingsManager::ManagerData SettingsManager::s_ManagerData;
SettingsManager::Registrar SettingsManager::s_Registrar;

SettingsManager::Registrar::Registrar() {
  s_ManagerData.Descriptors = {
      {"cloneOffset", "Clone Offset", SettingType::Float3, &s_Settings.cloneOffset},
      {"objImportScale", "OBJ Import Scale", SettingType::Float, &s_Settings.objImportScale},
      {"leftPaneWidth", "Left Pane Width", SettingType::Float, &s_Settings.leftPaneWidth},
      {"rightPaneWidth", "Right Pane Width", SettingType::Float, &s_Settings.rightPaneWidth},
      {"gridSize", "Grid Size", SettingType::Int, &s_Settings.gridSize},
      {"gridDivisions", "Grid Divisions", SettingType::Int, &s_Settings.gridDivisions},
      {"cameraSpeed", "Camera Speed", SettingType::Float, &s_Settings.cameraSpeed}
  };

  for (const auto& desc : s_ManagerData.Descriptors) {
    s_ManagerData.DescriptorMap[desc.key] = &desc;
  }
}

AppSettings& SettingsManager::Get() { return s_Settings; }

const std::vector<SettingDescriptor>& SettingsManager::GetDescriptors() {
  return s_ManagerData.Descriptors;
}

bool SettingsManager::Load(const std::string& path) {
  std::ifstream in(path);
  if (!in.is_open()) {
    return false;
  }

  try {
    const nlohmann::json j = nlohmann::json::parse(in);

    for (const auto& desc : GetDescriptors()) {
      if (j.contains(desc.key)) {
        const auto& jsonValue = j.at(desc.key);
        switch (desc.type) {
          case SettingType::Float:
            *static_cast<float*>(desc.ptr) = jsonValue.get<float>();
            break;
          case SettingType::Float3:
            *static_cast<glm::vec3*>(desc.ptr) = jsonValue.get<glm::vec3>();
            break;
          case SettingType::Int:
            *static_cast<int*>(desc.ptr) = jsonValue.get<int>();
            break;
        }
      }
    }
  } catch (const nlohmann::json::exception& e) {
    Log::Debug("Failed to parse settings.json: ", e.what());
    return false;
  }

  return true;
}

bool SettingsManager::Save(const std::string& path) {
  nlohmann::json j;

  for (const auto& desc : GetDescriptors()) {
    switch (desc.type) {
      case SettingType::Float:
        j[desc.key] = *static_cast<const float*>(desc.ptr);
        break;
      case SettingType::Float3:
        j[desc.key] = *static_cast<const glm::vec3*>(desc.ptr);
        break;
      case SettingType::Int:
        j[desc.key] = *static_cast<const int*>(desc.ptr);
        break;
    }
  }

  std::ofstream out(path);
  if (!out.is_open()) {
    return false;
  }

  out << std::setw(4) << j << std::endl;
  return true;
}