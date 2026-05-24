#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float threshold;

void main() {
    vec3 color = texture(screenTexture, TexCoords).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));

    float excess = max(brightness - threshold, 0.0);
    float factor = excess / (excess + 0.1); // zachte overgang

    if (brightness > threshold)
        FragColor = vec4(color * factor, 1.0); // gedimd naar mate van overschrijding
    else
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}