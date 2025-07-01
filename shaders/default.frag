#version 330 core
out vec4 FragColor;

// This line receives the color from your C++ code
uniform vec4 u_Color;

void main()
{
    // Sets the pixel's color to the value passed in
    FragColor = u_Color;
}