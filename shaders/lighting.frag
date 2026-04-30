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

uniform Light light;
uniform vec3 viewPos;
uniform sampler2D texture_diffuse1;
uniform bool hasDiffuseTexture;

void main() {
    vec3 albedo;
    if (hasDiffuseTexture) {
        albedo = texture(texture_diffuse1, TexCoords).rgb;
    } else {
        albedo = vec3(0.8, 0.8, 0.8);
    }

    vec3 ambient = light.ambient * albedo;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * albedo;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = light.specular * spec * 0.2;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}