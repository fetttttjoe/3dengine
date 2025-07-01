#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float outlineScale; // A scale factor for the outline

void main() {
    // Scale the position slightly for the outline effect
    gl_Position = projection * view * model * vec4(aPos * outlineScale, 1.0);
}