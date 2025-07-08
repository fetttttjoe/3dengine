#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>

/** Supported UI widget types for auto-generation. */
enum class SettingType {
  Float3,
  Float,
};

/** Descriptor for one setting. */
struct SettingDescriptor {
  std::string key;
  std::string label;
  SettingType type;
  void* ptr;
};

/** All of your configurable settings with defaults. */
struct AppSettings {
  glm::vec3 cloneOffset = {0.5f, 0.5f, 0.0f};
  float leftPaneWidth = 200.0f;
  float rightPaneWidth = 300.0f;
};

class SettingsManager {
 public:
  static AppSettings& Get();
  static bool Load(const std::string& path = "settings.json");
  static bool Save(const std::string& path = "settings.json");
  static const std::vector<SettingDescriptor>& GetDescriptors();

 private:
  // FIX: This struct holds the manager's internal data.
  // It was missing from the header file.
  struct ManagerData {
      std::vector<SettingDescriptor> Descriptors;
      std::unordered_map<std::string, const SettingDescriptor*> DescriptorMap;
  };
  
  static AppSettings s_Settings;
  static ManagerData s_ManagerData;

  // The Registrar uses a static instance to ensure its constructor
  // runs once at program startup to populate the settings data.
  struct Registrar {
    Registrar();
  };
  static Registrar s_Registrar;
};