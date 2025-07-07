#pragma once

// Forward declarations
class Scene;

class InspectorView {
 public:
  explicit InspectorView(
      Scene* scene);  // Needs Scene to get selected object properties
  void Draw();

 private:
  Scene* m_Scene;
};