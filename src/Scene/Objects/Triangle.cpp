// Scene/Objects/Triangle.cpp
#include "Triangle.h"

Triangle::Triangle() {
    m_TypeString = "Triangle";
    name = m_TypeString;
    // Now that m_TypeString (and any dims) are set, build the mesh:
    RebuildMesh();
}

void Triangle::BuildMeshData(std::vector<float>& vertices,
                             std::vector<unsigned int>& indices)
{
    // Simple 2D triangle in the X–Y plane at Z=0
    vertices = {
         0.0f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f
    };
    // single triangle
    indices = { 0, 1, 2 };
}
