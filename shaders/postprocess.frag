#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform int effect; // 0=none, 1=blur, 2=edge

const float offset = 1.0 / 300.0;

void main() {
    // Buur-offsets voor 3x3 kernel
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), vec2(0.0,  offset), vec2(offset,  offset),
        vec2(-offset,  0.0),    vec2(0.0,  0.0),    vec2(offset,  0.0),
        vec2(-offset, -offset), vec2(0.0, -offset), vec2(offset, -offset)
    );

    // Gaussian blur kernel
    float blur[9] = float[](
        1.0/16.0, 2.0/16.0, 1.0/16.0,
        2.0/16.0, 4.0/16.0, 2.0/16.0,
        1.0/16.0, 2.0/16.0, 1.0/16.0
    );

    // Laplacian edge detection kernel
    float edge[9] = float[](
        -1.0, -1.0, -1.0,
        -1.0,  8.0, -1.0,
        -1.0, -1.0, -1.0
    );

    if (effect == 0) {
        // Geen effect
        FragColor = texture(screenTexture, TexCoords);
    } else {
        // Sample 3x3 buren
        vec3 samples[9];
        for (int i = 0; i < 9; i++)
            samples[i] = vec3(texture(screenTexture, TexCoords + offsets[i]));

        // Kies kernel
        float kernel[9];
        if (effect == 1) {
            for (int i = 0; i < 9; i++) kernel[i] = blur[i];
        } else {
            for (int i = 0; i < 9; i++) kernel[i] = edge[i];
        }

        // Convolutie toepassen
        vec3 result = vec3(0.0);
        for (int i = 0; i < 9; i++)
            result += samples[i] * kernel[i];

        FragColor = vec4(result, 1.0);
    }
}