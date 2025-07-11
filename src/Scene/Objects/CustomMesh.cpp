#include "Scene/Objects/CustomMesh.h"
#include "Scene/Objects/ObjectTypes.h"

CustomMesh::CustomMesh() {
    name = "Custom Mesh";
    RebuildMesh();
}

CustomMesh::CustomMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices)
    : m_InitialVertices(vertices), m_InitialIndices(indices) {
    name = "Custom Mesh";
    RebuildMesh();
}

std::string CustomMesh::GetTypeString() const {
    return std::string(ObjectTypes::CustomMesh);
}

void CustomMesh::BuildMeshData(std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    vertices = m_InitialVertices;
    indices = m_InitialIndices;
}