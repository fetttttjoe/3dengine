#version 330 core
out vec4 FragColor;

// Uniform for passing a dynamic color from the C++ code
uniform vec4 u_Color;

void main()
{
    FragColor = u_Color;
}