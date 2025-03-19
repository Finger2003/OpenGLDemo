#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;
uniform bool isFoggy;
uniform vec3 fogColor;

void main()
{    
    FragColor = isFoggy ? vec4(fogColor, 1.0) : texture(skybox, TexCoords);
}