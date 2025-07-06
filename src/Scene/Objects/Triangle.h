#pragma once

#include "BaseObject.h"

class Triangle : public BaseObject {
 public:
  Triangle();
  ~Triangle() override = default;

  // ISceneObject override
  std::string GetTypeString() const override;

 protected:
  // BaseObject override
  void BuildMeshData(std::vector<float>& vertices,
                     std::vector<unsigned int>& indices) override;
};