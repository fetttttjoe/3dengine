#pragma once

#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vector>

/** Supported UI widget types for auto-generation. */
enum class SettingType {
  Float3,
  Float,
  Int
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
  // --- Object Settings ---
  glm::vec3 cloneOffset = {0.5f, 0.5f, 0.0f};
  float objImportScale = 1.0f;

  // --- UI Settings ---
  float leftPaneWidth = 200.0f;
  float rightPaneWidth = 300.0f;

  // --- World Settings ---
  int gridSize = 80;
  int gridDivisions = 80;
  float cameraSpeed = 5.0f;
};

class SettingsManager {
 public:
  static AppSettings& Get();
  static bool Load(const std::string& path = "settings.json");
  static bool Save(const std::string& path = "settings.json");
  static const std::vector<SettingDescriptor>& GetDescriptors();

 private:
  struct ManagerData {
    std::vector<SettingDescriptor> Descriptors;
    std::unordered_map<std::string, const SettingDescriptor*> DescriptorMap;
  };

  static AppSettings s_Settings;
  static ManagerData s_ManagerData;

  struct Registrar {
    Registrar();
  };
  static Registrar s_Registrar;
};