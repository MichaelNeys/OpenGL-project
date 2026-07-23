#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D overlayTexture;
uniform bool showOverlay;
uniform bool useChromaKey;

vec3 RGBtoYCbCr(vec3 rgb) {
    float Y  =  0.29900 * rgb.r + 0.58700 * rgb.g + 0.11400 * rgb.b;
    float Cb = -0.16874 * rgb.r - 0.33126 * rgb.g + 0.50000 * rgb.b + 0.5;
    float Cr =  0.50000 * rgb.r - 0.41869 * rgb.g - 0.08131 * rgb.b + 0.5;
    return vec3(Y, Cb, Cr);
}

void main() {
    if (!showOverlay) {
        discard;
    }

    vec4 texColor = texture(overlayTexture, TexCoords);

    if (useChromaKey) {
        // achtergrond kleur die we willen wegfilteren
        vec3 keyColorRGB = vec3(0.0, 1.0, 0.0); 

        vec3 pixelYCbCr = RGBtoYCbCr(texColor.rgb);
        vec3 keyYCbCr   = RGBtoYCbCr(keyColorRGB);

        // Afstand berekenen in het CbCr kleurenvlak
        float chromaDistance = distance(pixelYCbCr.yz, keyYCbCr.yz);
        
        // drempelwaarde het groen
        float threshold = 0.22; 

        // filter green
        if (chromaDistance < threshold) {
            discard;
        }
    }

    FragColor = texColor;
}