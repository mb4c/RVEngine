#version 460 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out uvec3 EntityColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in mat3 TBN;

layout(binding=0) uniform sampler2D albedoMap;
layout(binding=1) uniform sampler2D normalMap;
layout(binding=2) uniform sampler2D occlusionRoughnessMetallic;

layout(binding=3) uniform samplerCube irradianceMap;
layout(binding=4)  uniform samplerCube prefilterMap;
layout(binding=5)  uniform sampler2D brdfLUT;

// lights
#define MAX_LIGHTS 32
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightColors[MAX_LIGHTS];

uniform vec3 u_CamPos;
uniform uint u_ObjectIndex;
uniform uint u_DisplayType;

uniform bool u_UseAlbedo;
uniform bool u_UseNormal;
uniform bool u_UseORM;

uniform vec4 u_AlbedoColor;
uniform float u_RoughnessVal;
uniform float u_MetallicVal;


const float PI = 3.14159265359;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz;
    tangentNormal = tangentNormal * 2.0 - 1.0;

    return normalize(TBN * tangentNormal);
}

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
void main()
{
    vec3 albedo = u_AlbedoColor.rgb;
    vec3 N = Normal;
    float ao = 1;
    float metallic = u_MetallicVal;
    float roughness = u_RoughnessVal;

    if(u_UseAlbedo)
    {
        albedo = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));
    }
    if(u_UseNormal)
    {
        N = getNormalFromMap();
    }
    if(u_UseORM)
    {
        ao = texture(occlusionRoughnessMetallic, TexCoords).r;
        metallic  = texture(occlusionRoughnessMetallic, TexCoords).b;
        roughness = texture(occlusionRoughnessMetallic, TexCoords).g;
    }

    vec3 V = normalize(u_CamPos - WorldPos);
    vec3 R = reflect(-V, N);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < MAX_LIGHTS; ++i)
    {
        // calculate per-light radiance
        vec3 L = normalize(lightPositions[i] - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;

        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    // ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;


    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse      = irradiance * albedo;

    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;
    float NdotV = max(dot(N, V), 0.0);
    vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
//    vec2 brdf = texture(brdfLUT, vec2(NdotV, roughness)).rg;
//    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    vec3 specular = prefilteredColor * (F0 * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;


    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    switch (u_DisplayType)
    {
        case 0: // LIT
            FragColor = vec4(color, 1.0);
            break;
        case 1: // ALBEDO
            FragColor = vec4(albedo, 1.0);
            break;
        case 2: // AO
            FragColor = vec4(vec3(ao), 1.0);
            break;
        case 3: // Rougness
            FragColor = vec4(vec3(roughness), 1.0);
            break;
        case 4: // Metallic
            FragColor = vec4(vec3(metallic), 1.0);
            break;
        case 5: // Vertex normal
            FragColor = vec4(Normal * 0.5 + 0.5, 1.0);
            break;
        case 6: // World position
            FragColor = vec4(WorldPos, 1.0);
            break;
        case 7: // Tangent normal
            FragColor = vec4(texture(normalMap, TexCoords).xyz, 1.0);
            break;
        case 8: // TBN * vec3(1)
            FragColor = vec4(TBN * vec3(1), 1.0);
            break;
        case 9: // Normal mapped
            FragColor = vec4(getNormalFromMap() * 0.5 + 0.5, 1.0);
            break;
        case 10: // Specular
            FragColor = vec4(specular, 1.0);
            break;
        case 11: // ambient
            FragColor = vec4(ambient, 1.0);
            break;
        case 12: // prefiltered color
            FragColor = vec4(prefilteredColor, 1.0);
            break;
        case 13: // diffuse
            FragColor = vec4(diffuse, 1.0);
            break;
        case 14: // r
            FragColor = vec4(R, 1.0);
            break;
        case 15: // v
            FragColor = vec4(V, 1.0);
            break;
    }



    EntityColor = uvec3(u_ObjectIndex, 0, gl_PrimitiveID);
}
