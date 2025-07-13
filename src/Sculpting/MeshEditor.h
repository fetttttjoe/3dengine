#pragma once

#include "Interfaces/IEditableMesh.h"
#include "Sculpting/SubObjectSelection.h"

class MeshEditor {
 public:
  void Extrude(IEditableMesh& mesh, const SubObjectSelection& selection,
               float distance);
  void Weld(IEditableMesh& mesh, SubObjectSelection& selection);
  void MoveAlongNormal(IEditableMesh& mesh, const SubObjectSelection& selection,
                       float distance);
};