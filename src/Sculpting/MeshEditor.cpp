#include "Sculpting/MeshEditor.h"

#include "Core/Log.h"
#include "Interfaces/IEditableMesh.h"
#include "Sculpting/SubObjectSelection.h"

void MeshEditor::Extrude(IEditableMesh& mesh,
                         const SubObjectSelection& selection, float distance) {
  if (selection.GetSelectedFaces().empty()) return;
  mesh.ExtrudeFaces(selection.GetSelectedFaces(), distance);
}

void MeshEditor::Weld(IEditableMesh& mesh, SubObjectSelection& selection) {
  const auto& selectedVertices = selection.GetSelectedVertices();
  if (selectedVertices.size() < 2) return;

  glm::vec3 weldPoint(0.0f);
  for (uint32_t index : selectedVertices) {
    weldPoint += mesh.GetVertices()[index];
  }
  weldPoint /= selectedVertices.size();

  mesh.WeldVertices(selectedVertices, weldPoint);
  selection.Clear();
}

void MeshEditor::BevelEdges(IEditableMesh& mesh,
                            const SubObjectSelection& selection, float amount) {
    if (selection.GetSelectedEdges().empty()) return;
    mesh.BevelEdges(selection.GetSelectedEdges(), amount);
}

void MeshEditor::MoveAlongNormal(IEditableMesh& mesh,
                                 const SubObjectSelection& selection,
                                 float distance) {
  const auto& selectedVertices = selection.GetSelectedVertices();
  if (selectedVertices.empty()) return;

  glm::vec3 averageNormal(0.0f);
  for (uint32_t index : selectedVertices) {
    averageNormal += mesh.GetNormals()[index];
  }
  averageNormal = glm::normalize(averageNormal);

  for (uint32_t index : selectedVertices) {
    mesh.GetVertices()[index] += averageNormal * distance;
  }
  mesh.RecalculateNormals();
}
