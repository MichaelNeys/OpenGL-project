#version 330 core

in  vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D overlayTexture;
uniform bool      showOverlay;

vec3 RGBtoYCbCr(vec3 rgb) {
    float Y  =  0.299  * rgb.r + 0.587  * rgb.g + 0.114  * rgb.b;
    float Cb = -0.1687 * rgb.r - 0.3313 * rgb.g + 0.5    * rgb.b + 0.5;
    float Cr =  0.5    * rgb.r - 0.4187 * rgb.g - 0.0813 * rgb.b + 0.5;
    return vec3(Y, Cb, Cr);
}

void main() {
    if (!showOverlay) {
        // Toon puur blauw vlak als overlay uit is
        FragColor = vec4(0.0, 0.0, 1.0, 1.0);
        return;
    }

    vec4 texColor = texture(overlayTexture, TexCoords);

    vec3 chromaColor  = vec3(0.0, 0.0, 1.0);
    vec3 pixelYCbCr   = RGBtoYCbCr(texColor.rgb);
    vec3 chromaYCbCr  = RGBtoYCbCr(chromaColor);

    float diffCb     = abs(pixelYCbCr.y - chromaYCbCr.y);
    float diffCr     = abs(pixelYCbCr.z - chromaYCbCr.z);
    float chromaDiff = diffCb + diffCr;
    float saturation = length(texColor.rgb - vec3(dot(texColor.rgb, vec3(0.333))));

    if (chromaDiff < 0.15 && saturation > 0.2)
        discard;

    FragColor = texColor;
}