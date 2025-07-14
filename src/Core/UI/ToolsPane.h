#pragma once

#include "Core/UI/IView.h"

// Forward declarations
class Application;

class ToolsPane : public IView {
 public:
  explicit ToolsPane(Application* app);

  void Draw() override;
  const char* GetName() const override { return "ToolsPane"; }

 private:
  Application* m_App;
};