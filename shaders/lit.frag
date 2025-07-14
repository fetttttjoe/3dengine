#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec4 u_Color;

void main()
{
    // A simple directional light from a fixed direction
    vec3 lightDir = normalize(vec3(-0.5, -1.0, -0.3));
    
    // Ambient term
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * u_Color.rgb;
    
    // Diffuse term
    float diff = max(dot(normalize(Normal), -lightDir), 0.0);
    vec3 diffuse = diff * u_Color.rgb;
    
    vec3 result = ambient + diffuse;
    FragColor = vec4(result, u_Color.a);
}
