#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

/** Supported UI widget types for auto‐generation. */
enum class SettingType {
  Float3,
  Float,  // single float
  // ... you can add Int, Bool, etc.
};

/** Descriptor for one setting. */
struct SettingDescriptor {
  std::string key;    // JSON key
  std::string label;  // UI label
  SettingType type;
  void* ptr;  // pointer into AppSettings
};

/** All of your configurable settings with defaults. */
struct AppSettings {
  glm::vec3 cloneOffset = {0.5f, 0.5f, 0.0f};
  float leftPaneWidth = 200.0f;
  float rightPaneWidth = 300.0f;
  // … future fields here …
};

class SettingsManager {
 public:
  /// Get the singleton settings object
  static AppSettings& Get();

  /// Load from disk (settings.json). Returns false if missing or parse error.
  static bool Load(const std::string& path = "settings.json");

  /// Save to disk. Returns false on error.
  static bool Save(const std::string& path = "settings.json");

  /// List of all descriptors for auto‐UI.
  static const std::vector<SettingDescriptor>& GetDescriptors();

 private:
  static AppSettings s_Settings;
  static std::vector<SettingDescriptor> s_Descriptors;
  struct Registrar {
    Registrar();
  };
  static Registrar s_Registrar;
};
