#pragma once

#include "Core/UI/IView.h"

// Forward declarations
class Application;
class AppUI;

class SettingsWindow : public IView {
 public:
  explicit SettingsWindow(Application* app);

  void Draw() override;
  const char* GetName() const override { return "SettingsWindow"; }

  // These methods allow the AppUI to get and modify the live preview values
  bool IsVisible() const { return m_IsVisible; }
  float& GetLeftPaneWidth() {
    return m_TempLeftPaneWidth;
  }  // Changed to return float&
  float& GetRightPaneWidth() {
    return m_TempRightPaneWidth;
  }  // Changed to return float&

 private:
  void RevertToSavedSettings();  // Resets changes if the user cancels

  Application* m_App;
  AppUI* m_AppUI;  // Cache a pointer to the main UI

  // State for the settings window itself
  bool m_IsVisible = false;

  // Temporary state for live previewing of settings
  float m_TempLeftPaneWidth;
  float m_TempRightPaneWidth;
};