#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main() {
    TexCoords = aTexCoords;
    // Gebruik de exacte coördinaten uit de VBO (geen transformatiematrices nodig)
    gl_Position = vec4(aPos, 1.0); 
}