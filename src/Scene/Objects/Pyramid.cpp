#include "Pyramid.h"

Pyramid::Pyramid() {
    name = "Pyramid";
    RebuildMesh();
}

std::string Pyramid::GetTypeString() const {
    return "Pyramid";
}

void Pyramid::BuildMeshData(std::vector<float>& vertices,
                            std::vector<unsigned int>& indices)
{
    // Dimensions are taken from BaseObject members
    float w = m_Width  * 0.5f;
    float h = m_Height;
    float d = m_Depth  * 0.5f;

    // A pyramid with 5 vertices: 4 for the base, 1 for the apex.
    vertices = {
        // Base vertices
        -w, 0.0f, -d, // 0
         w, 0.0f, -d, // 1
         w, 0.0f,  d, // 2
        -w, 0.0f,  d, // 3
        // Apex vertex
         0,   h,    0  // 4
    };

    // 6 triangles: 2 for the base quad, 4 for the sides.
    indices = {
        // Base
        0, 1, 2,   2, 3, 0,
        // Sides
        0, 4, 1, // Front face
        1, 4, 2, // Right face
        2, 4, 3, // Back face
        3, 4, 0  // Left face
    };
}