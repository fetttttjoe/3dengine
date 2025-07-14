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
  void DrawMaterialControls(ISceneObject* sel);
  void DrawProperties(ISceneObject* sel);
  void DrawMeshEditingControls(ISceneObject* sel);
  void DrawBrushSettings();
  void DrawSubObjectSettings();

  // Helpers to determine if tools should be active
  bool CanWeld() const;
  bool CanExtrude() const;
  bool CanBevel() const; // New helper for the edge tool

  Application* m_App;
  Scene* m_Scene;

  BrushSettings m_BrushSettings;

  // State for sub-object tools
  float m_ExtrudeDistance = 0.1f;
  float m_BevelAmount = 0.1f;
};