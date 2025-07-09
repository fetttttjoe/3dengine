#pragma once

#include "Core/UI/IView.h"

// Forward declarations
class Application;

class MenuBar : public IView {
 public:
  explicit MenuBar(Application* app);

  void Draw() override;
  const char* GetName() const override { return "MenuBar"; }

 private:
  void DrawFileMenu();
  void DrawViewMenu();
  void DrawSceneMenu();
  void DrawAddObjectSubMenu();

  Application* m_App;
};
