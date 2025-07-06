#include "Core/SettingsManager.h"

#include <fstream>
#include <iomanip>

#include "nlohmann/json.hpp"

AppSettings SettingsManager::s_Settings;
std::vector<SettingDescriptor> SettingsManager::s_Descriptors;
SettingsManager::Registrar SettingsManager::s_Registrar;

SettingsManager::Registrar::Registrar() {
  // Register each setting in order
  s_Descriptors.push_back({"cloneOffset", "Clone Offset", SettingType::Float3,
                           &s_Settings.cloneOffset});
  s_Descriptors.push_back({"leftPaneWidth", "Left Pane Width",
                           SettingType::Float, &s_Settings.leftPaneWidth});
  s_Descriptors.push_back({"rightPaneWidth", "Right Pane Width",
                           SettingType::Float, &s_Settings.rightPaneWidth});
}

AppSettings& SettingsManager::Get() { return s_Settings; }

bool SettingsManager::Load(const std::string& path) {
  std::ifstream in(path);
  if (!in.is_open()) return false;
  nlohmann::json j;
  in >> j;

  if (auto it = j.find("cloneOffset"); it != j.end() && it->is_object()) {
    auto ptr = reinterpret_cast<glm::vec3*>(s_Descriptors[0].ptr);
    ptr->x = (*it).value("x", ptr->x);
    ptr->y = (*it).value("y", ptr->y);
    ptr->z = (*it).value("z", ptr->z);
  }
  if (auto it = j.find("leftPaneWidth"); it != j.end())
    s_Settings.leftPaneWidth = *it;
  if (auto it = j.find("rightPaneWidth"); it != j.end())
    s_Settings.rightPaneWidth = *it;

  return true;
}

bool SettingsManager::Save(const std::string& path) {
  nlohmann::json j;
  {
    auto ptr = reinterpret_cast<glm::vec3*>(s_Descriptors[0].ptr);
    j["cloneOffset"] = {{"x", ptr->x}, {"y", ptr->y}, {"z", ptr->z}};
  }
  j["leftPaneWidth"] = s_Settings.leftPaneWidth;
  j["rightPaneWidth"] = s_Settings.rightPaneWidth;

  std::ofstream out(path);
  if (!out.is_open()) return false;
  out << std::setw(4) << j << std::endl;
  return true;
}

const std::vector<SettingDescriptor>& SettingsManager::GetDescriptors() {
  return s_Descriptors;
}
