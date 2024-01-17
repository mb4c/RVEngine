#version 330 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;

uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 WorldPos;

void main()
{
    WorldPos = a_Position;

    mat4 rotView = mat4(mat3(u_View));
    vec4 clipPos = u_Projection * rotView * vec4(WorldPos, 1.0);

    gl_Position = clipPos.xyww;
}
