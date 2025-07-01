#version 330 core
out vec4 FragColor;

uniform vec4 outlineColor; // Color for the outline

void main() {
    FragColor = outlineColor;
}