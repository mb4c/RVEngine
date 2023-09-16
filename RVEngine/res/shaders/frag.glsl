#version 330 core

layout(location = 0) out vec4 color;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light
{
//    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 u_CamPos;
uniform Material u_Material;
uniform Light u_Light;

void main()
{
    // ambient
    vec3 ambient = u_Light.ambient * texture(u_Material.diffuse, TexCoords).rgb;

    // diffuse
    vec3 norm = normalize(Normal);
//    vec3 lightDir = normalize(u_Light.position - FragPos);
    vec3 lightDir = normalize(-u_Light.direction);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = u_Light.diffuse * diff * texture(u_Material.diffuse, TexCoords).rgb;

    // specular
    vec3 viewDir = normalize(u_CamPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.shininess);
    vec3 specular = u_Light.specular * texture(u_Material.specular, TexCoords).rgb;

    vec3 result = ambient + diffuse + specular;
    color = vec4(result, 1.0);

}