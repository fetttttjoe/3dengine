#version 330 core
layout (location = 0) in vec3 aPos;

// Uniforms are variables passed from the C++ code to the shader
uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    // Transform the vertex position from model space to world, then view, then clip space
    gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 1.0);
}