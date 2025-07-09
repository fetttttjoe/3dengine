#pragma once

#include "Core/UI/IView.h"
#include "Sculpting/ISculptTool.h"

// Forward declarations
class Application;
class Scene;
class ISceneObject;

class InspectorView : public IView {
 public:
  explicit InspectorView(Application* app);

  void Draw() override;
  const char* GetName() const override { return "InspectorView"; }

  float GetBrushRadius() const { return m_BrushRadius; }
  float GetBrushStrength() const { return m_BrushStrength; }

 private:
  void DrawTransformControls(ISceneObject* sel);
  void DrawProperties(ISceneObject* sel);
  void DrawSculptControls(ISceneObject* sel);

  Application* m_App;
  Scene* m_Scene;

  float m_BrushRadius = 0.5f;
  float m_BrushStrength = 0.5f;
};
