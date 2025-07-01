// Scene/Objects/Pyramid.cpp
#include "Pyramid.h"

Pyramid::Pyramid() {
    m_TypeString = "Pyramid";
    name = m_TypeString;
    // If you like, you can tweak m_Width/m_Height/m_Depth here before building
    RebuildMesh();
}

void Pyramid::BuildMeshData(std::vector<float>& vertices,
                            std::vector<unsigned int>& indices)
{
    float w = m_Width  * 0.5f;
    float h = m_Height;
    float d = m_Depth  * 0.5f;

    // base: 4 verts, plus apex at index 4
    vertices = {
        -w, 0.0f, -d,
         w, 0.0f, -d,
         w, 0.0f,  d,
        -w, 0.0f,  d,
         0,   h,    0
    };

    // two triangles for base + 4 side faces
    indices = {
        0,1,2,   2,3,0,  // base
        0,1,4,           // front
        1,2,4,           // right
        2,3,4,           // back
        3,0,4            // left
    };
}
