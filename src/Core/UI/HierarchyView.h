#pragma once

#include <Interfaces.h>

#include <cstdint>
#include <functional>
#include <string>
// Forward declarations
class Scene;
class Application;  // Needed for SelectObject, GetTransformGizmo

class HierarchyView {
 public:
  explicit HierarchyView(
      Application* app,
      Scene* scene);  // Needs both App and Scene for interactions

  void Draw();

  // Callbacks for actions that affect the application/scene
  std::function<void(uint32_t)> OnObjectSelected;
  std::function<void(uint32_t)> OnObjectDeleted;
  std::function<void(uint32_t)> OnObjectDuplicated;

 private:
  Application* m_App;
  Scene* m_Scene;

  uint32_t m_RenameID = 0;
  std::string m_RenameBuffer;
  uint32_t m_IdToDelete = 0;
};