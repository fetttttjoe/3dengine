#pragma once

#include "Core/Application.h"
#include "Core/UI/BrushSettings.h"
#include "Core/UI/IView.h"

class Application;
class Scene;
class ISceneObject;

class InspectorView : public IView {
 public:
  explicit InspectorView(Application* app);

  void Draw() override;
  const char* GetName() const override { return "InspectorView"; }

  const BrushSettings& GetBrushSettings() const { return m_BrushSettings; }
  BrushSettings& GetBrushSettings() { return m_BrushSettings; }

 private:
  void DrawTransformControls(ISceneObject* sel);
  void DrawProperties(ISceneObject* sel);
  void DrawBrushSettings();
  void DrawSubObjectSettings();

  Application* m_App;
  Scene* m_Scene;

  BrushSettings m_BrushSettings;

  // State for new tools
  float m_ExtrudeDistance = 0.1f;
  float m_MoveDistance = 0.1f;
};