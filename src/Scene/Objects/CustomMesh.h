#pragma once

#include "Scene/Objects/BaseObject.h"

class CustomMesh : public BaseObject {
public:
    CustomMesh();
    CustomMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices);
    ~CustomMesh() override = default;

    std::string GetTypeString() const override;

protected:
    void BuildMeshData(std::vector<float>& vertices, std::vector<unsigned int>& indices) override;

private:
    std::vector<float> m_InitialVertices;
    std::vector<unsigned int> m_InitialIndices;
};