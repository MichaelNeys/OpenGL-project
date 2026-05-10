#version 330 core
out vec4 FragColor;
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Structuur voor een Lantaarn (Point Light)
struct PointLight {
    vec3 position;

    // Afzwakking over afstand (Attenuation)
    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Bepaal hoeveel lantaarns je maximaal wilt ondersteunen (bijv. 4)
#define NR_POINT_LIGHTS 4

uniform Light light;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform vec3 viewPos;
uniform sampler2D texture_diffuse1;
uniform bool hasDiffuseTexture;

// Functiedeclaratie voor het berekenen van 1 lantaarn
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo);

void main() {
    vec4 texColor = hasDiffuseTexture 
        ? texture(texture_diffuse1, TexCoords) 
        : vec4(0.8, 0.8, 0.8, 1.0);

    if (texColor.a < 0.5) discard;  // transparante pixels weggooien

    vec3 albedo = texColor.rgb;
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // 1. Basislicht berekenen (Het maanlicht)
    vec3 ambient = light.ambient * albedo;
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * albedo;
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = light.specular * spec * 0.2;
    
    vec3 result = ambient + diffuse + specular;

    // 2. Lantaarns toevoegen aan het resultaat!
    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, albedo);    
    }

    FragColor = vec4(result, texColor.a);
}

// De wiskunde voor het uitstralen en afzwakken van lantaarnlicht
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Diffuse 
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    
    // Attenuation (Hoe verder weg, hoe zwakker het licht)
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    
    // Combineer
    vec3 ambient = light.ambient * albedo;
    vec3 diffuse = light.diffuse * diff * albedo;
    vec3 specular = light.specular * spec * 0.2;
    
    return (ambient + diffuse + specular) * attenuation;
}