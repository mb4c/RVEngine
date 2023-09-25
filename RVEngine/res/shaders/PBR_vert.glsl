#version 330 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;
uniform mat3 u_NormalMatrix;

out vec3 Normal;
out vec2 TexCoords;
out vec3 WorldPos;

void main()
{
    TexCoords = a_TexCoord;
    WorldPos = vec3(u_Transform * vec4(a_Position, 1.0));
    Normal = u_NormalMatrix * a_Normal;

    gl_Position =  u_ViewProjection * vec4(WorldPos, 1.0);
}
