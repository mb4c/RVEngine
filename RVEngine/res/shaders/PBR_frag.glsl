#version 460 core

layout(location = 0) out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in mat3 TBN;

layout(binding=0) uniform sampler2D albedoMap;
layout(binding=1) uniform sampler2D normalMap;
layout(binding=2) uniform sampler2D occlusionMap;
layout(binding=3) uniform sampler2D roughnessMap;
layout(binding=4) uniform sampler2D metallicMap;
layout(binding=5) uniform sampler2D emissionMap;

layout(binding=6) uniform samplerCube irradianceMap;
layout(binding=7)  uniform samplerCube prefilterMap;
layout(binding=8)  uniform sampler2D brdfLUT;

// lights
#define MAX_LIGHTS 32
uniform int numLights;
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightDirections[MAX_LIGHTS];
uniform vec3 lightColors[MAX_LIGHTS];
uniform int lightTypes[MAX_LIGHTS];// 0 = point, 1 = directional

uniform vec3 u_CamPos;
uniform uint u_ObjectIndex;
uniform uint u_DisplayType;

layout(std140, binding = 9) uniform MaterialUBO
{
    vec4  albedoColor;// u_AlbedoColor
    vec4  emissionColor;// u_EmissionColor
    vec4  params;// x=roughness, y=metallic, z=emissionStrength, w=flags
};


const float PI = 3.14159265359;

bool HasFlag(int mask, int bit) { return (mask & (1 << bit)) != 0; }

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz;
    tangentNormal = tangentNormal * 2.0 - 1.0;

    return normalize(TBN * tangentNormal);
}

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

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 PBRNeutralToneMapping(vec3 color)
{
    const float startCompression = 0.8 - 0.04;
    const float desaturation = 0.15;

    float x = min(color.r, min(color.g, color.b));
    float offset = x < 0.08 ? x - 6.25 * x * x : 0.04;
    color -= offset;

    float peak = max(color.r, max(color.g, color.b));
    if (peak < startCompression) return color;

    const float d = 1. - startCompression;
    float newPeak = 1. - d * d / (peak + d - startCompression);
    color *= newPeak / peak;

    float g = 1. - 1. / (desaturation * (peak - newPeak) + 1.);
    return mix(color, newPeak * vec3(1, 1, 1), g);
}

void main()
{
    vec3 N = Normal;
    float ao = 1;

    uint flags = floatBitsToUint(params.w);

    bool useAlbedo    = (flags & (1u << 0u)) != 0u;
    bool useNormal    = (flags & (1u << 1u)) != 0u;
    bool useOcclusion = (flags & (1u << 2u)) != 0u;
    bool useRoughness = (flags & (1u << 3u)) != 0u;
    bool useMetallic  = (flags & (1u << 4u)) != 0u;
    bool useEmission  = (flags & (1u << 5u)) != 0u;
    bool useIBL       = (flags & (1u << 6u)) != 0u;


    vec3 albedo = albedoColor.rgb;
    float metallic = params.y;// from UBO
    float roughness = params.x;// from UBO
    float alpha = texture(albedoMap, TexCoords).a;

    if (alpha < 0.75)
    discard;

    if (useAlbedo) {
        vec4 albedoSample = texture(albedoMap, TexCoords);
        // divide by alpha to remove premultiplied darkening
        albedo = pow(albedoSample.rgb / max(albedoSample.a, 0.001), vec3(2.2));
        //albedo = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));
    }
    if (useNormal) {
        N = getNormalFromMap();
    }
    if (useOcclusion) {
        ao = texture(occlusionMap, TexCoords).r;
    }
    if (useRoughness) {
        roughness = texture(roughnessMap, TexCoords).g;
    }
    if (useMetallic) {
        metallic = texture(metallicMap, TexCoords).b;
    }

    vec3 V = normalize(u_CamPos - WorldPos);
    vec3 R = reflect(-V, N);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < numLights; ++i)
    {
        vec3 L;
        vec3 radiance;
        float distance = 1.0;

        if (lightTypes[i] == 0)// Point light
        {
            vec3 toLight = lightPositions[i] - WorldPos;
            distance = length(toLight);
            L = normalize(toLight);
            radiance = lightColors[i] / (distance * distance);
        }
        else // Directional light
        {
            L = normalize(-lightDirections[i]);
            radiance = lightColors[i];
        }

        // Correct normal to face the view
        vec3 N_corrected = (dot(N, V) < 0.0) ? -N : N;

        // Half-vector
        vec3 H = normalize(V + L);

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N_corrected, H, roughness);
        float G   = GeometrySmith(N_corrected, V, L, roughness);
        vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N_corrected, V), 0.0) * max(dot(N_corrected, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        // Diffuse and specular components
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N_corrected, L), 0.0);

        // Accumulate radiance
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 ambient = vec3(0.0);
    vec3 irradiance = vec3(0.0);
    vec3 diffuse = vec3(0.0);
    vec3 prefilteredColor = vec3(0.0);
    vec3 specular = vec3(0.0);

    if (useIBL)
    {
        irradiance = texture(irradianceMap, N).rgb;
        diffuse = irradiance * albedo;

        // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
        float NdotV = max(dot(N, V), 0.0);
        vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
        //    vec2 brdf = texture(brdfLUT, vec2(NdotV, roughness)).rg;
        //            vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
        specular = prefilteredColor * (F0 * brdf.x + brdf.y);
        ambient = (kD * diffuse + specular) * ao;
    }
    else
    {
        ambient = vec3(0.03) * albedo * ao;
    }
    vec3 emission = vec3(0.0);
    if (useEmission) {
        emission = texture(emissionMap, TexCoords).rgb * emissionColor.rgb * params.z;
    }
    vec3 color = ambient + Lo + emission;


    // HDR tonemapping
    color = PBRNeutralToneMapping(color);
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    switch (u_DisplayType)
    {
        case 0:// LIT
        FragColor = vec4(color * alpha, alpha);
        break;
        case 1:// ALBEDO
        FragColor = vec4(albedo, 1.0);
        break;
        case 2:// AO
        FragColor = vec4(vec3(ao), 1.0);
        break;
        case 3:// Rougness
        FragColor = vec4(vec3(roughness), 1.0);
        break;
        case 4:// Metallic
        FragColor = vec4(vec3(metallic), 1.0);
        break;
        case 5:// Vertex normal
        FragColor = vec4(Normal * 0.5 + 0.5, 1.0);
        break;
        case 6:// World position
        FragColor = vec4(WorldPos, 1.0);
        break;
        case 7:// Tangent normal
        FragColor = vec4(texture(normalMap, TexCoords).xyz, 1.0);
        break;
        case 8:// TBN * vec3(1)
        FragColor = vec4(TBN * vec3(1), 1.0);
        break;
        case 9:// Normal mapped
        FragColor = vec4(getNormalFromMap() * 0.5 + 0.5, 1.0);
        break;
        case 10:// Specular
        FragColor = vec4(specular, 1.0);
        break;
        case 11:// ambient
        FragColor = vec4(ambient, 1.0);
        break;
        case 12:// prefiltered color
        FragColor = vec4(prefilteredColor, 1.0);
        break;
        case 13:// diffuse
        FragColor = vec4(diffuse, 1.0);
        break;
        case 14:// r
        FragColor = vec4(R, 1.0);
        break;
        case 15:// v
        FragColor = vec4(V, 1.0);
        break;
    }
}
