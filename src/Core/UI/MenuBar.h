#pragma once

#include <functional>
#include <string>

class Application;
class Scene;
class SceneObjectFactory;

class MenuBar {
 public:
  explicit MenuBar(Application* app, Scene* scene, SceneObjectFactory* factory);

  void Draw();

  // Getter for the factory (needed by AppUI's lambda)
  SceneObjectFactory* GetFactory() const;  // <-- REMOVE THE BODY HERE

  // Callbacks for events that affect the application or scene
  std::function<void()> OnExitRequested;
  std::function<void()> OnSaveScene;
  std::function<void()> OnLoadScene;
  std::function<void()> OnShowSettings;
  std::function<void(bool)> OnShowAnchorsChanged;
  std::function<void()> OnDeleteSelectedObject;
  std::function<void(const std::string&)> OnAddObject;

 private:
  Application* m_App;
  Scene* m_Scene;
  SceneObjectFactory* m_Factory;

  void DrawFileMenu();
  void DrawViewMenu();
  void DrawSceneMenu();
  void DrawAddObjectSubMenu();
};