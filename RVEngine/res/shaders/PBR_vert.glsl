#version 460 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;
layout (location = 3) in vec3 a_Tangent;
layout (location = 4) in vec3 a_Bitangent;

uniform mat4 u_ViewProjection;
uniform mat4 u_View;
uniform mat4 u_Transform;
uniform mat3 u_NormalMatrix;

out vec3 Normal;
out vec2 TexCoords;
out vec3 WorldPos;
out mat3 TBN;

void main()
{
    TexCoords = a_TexCoord;
//    WorldPos = vec3(u_Transform * vec4(a_Position, 1.0));
    WorldPos = (u_Transform * vec4(a_Position, 1.0)).xyz;

    Normal = normalize(u_NormalMatrix * a_Normal);

    vec3 T = normalize(vec3(u_NormalMatrix * a_Tangent));
    vec3 B = normalize(vec3(u_NormalMatrix * a_Bitangent));
    vec3 N = normalize(vec3(u_NormalMatrix * a_Normal));
    TBN = mat3(T, B, N);

    gl_Position =  u_ViewProjection * vec4(WorldPos, 1.0);
}
