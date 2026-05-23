#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D texture_diffuse1;

void main() {
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    if (texColor.a < 0.5) discard;
    
    // Alleen lichtbronnen krijgen HDR boost voor bloom
    FragColor = vec4(texColor.rgb * 2.0, texColor.a);
}