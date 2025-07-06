#include "Triangle.h"

Triangle::Triangle() {
    name = "Triangle";
    // This object is 2D, so depth is irrelevant.
    m_Depth = 0.0f;
    RebuildMesh();
}

std::string Triangle::GetTypeString() const {
    return "Triangle";
}

void Triangle::BuildMeshData(std::vector<float>& vertices,
                             std::vector<unsigned int>& indices)
{
    // Dimensions are taken from BaseObject members
    float w = m_Width * 0.5f;
    float h = m_Height * 0.5f;

    // A simple triangle in the X-Y plane.
    vertices = {
         0.0f,    h, 0.0f,
           -w,   -h, 0.0f,
            w,   -h, 0.0f
    };
    
    indices = { 0, 1, 2 };
}