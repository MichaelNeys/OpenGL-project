#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform float bloomStrength;
uniform bool bloomEnabled;

void main() {
    vec3 sceneColor = texture(scene, TexCoords).rgb;
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    
    // Voeg alleen de bloom gloed toe, geen tone mapping
    if (bloomEnabled)
        sceneColor += bloomColor * bloomStrength;

    FragColor = vec4(sceneColor, 1.0);
}