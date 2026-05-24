#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform bool hasDiffuseTexture;
uniform vec3 fallbackColor;

void main() {
    vec4 texColor;
    
    if (hasDiffuseTexture) {
        texColor = texture(texture_diffuse1, TexCoords);
        if (texColor.a < 0.5) discard;
    } else {
        texColor = vec4(fallbackColor, 1.0);
    }

    FragColor = vec4(texColor.rgb * 2.0, texColor.a);
}