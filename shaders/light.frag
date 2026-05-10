#version 330 core
out vec4 FragColor;

in vec2 TexCoords; // Ontvangen vanuit de vertex shader

uniform sampler2D texture_diffuse1; // Jouw C++ Model-lader vult deze automatisch in!

void main()
{
    // Lees de kleur van de textuur af
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    
    // Zorg ervoor dat onzichtbare/transparante pixels in Minecraft ook echt onzichtbaar zijn
    if (texColor.a < 0.5) {
        discard; 
    }
    
    // Output de pure textuurkleur (zonder schaduw, zodat hij "glowt")
    FragColor = texColor;
}