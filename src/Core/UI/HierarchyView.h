#pragma once

#include <cstdint>
#include <string>

#include "Core/UI/IView.h"

// Forward declarations
class Application;
class Scene;

class HierarchyView : public IView {
 public:
  explicit HierarchyView(Application* app);

  void Draw() override;
  const char* GetName() const override { return "HierarchyView"; }

 private:
  Application* m_App;
  Scene* m_Scene;

  uint32_t m_RenameID = 0;
  std::string m_RenameBuffer;
};
