#pragma once

#include "Core/UI/IView.h"
#include "Core/UI/BrushSettings.h"

// Forward declarations
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
    void DrawSculptControls(ISceneObject* sel);
    void DrawBrushSettings();

    Application* m_App;
    Scene* m_Scene;

    BrushSettings m_BrushSettings;
};