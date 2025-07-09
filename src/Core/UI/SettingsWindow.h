#pragma once

#include "Core/UI/IView.h"

// Forward declarations
class Application;

class SettingsWindow : public IView {
 public:
  explicit SettingsWindow(Application* app);

  void Draw() override;
  const char* GetName() const override { return "SettingsWindow"; }

 private:
  Application* m_App;
};
