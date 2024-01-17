#version 330 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;

uniform mat4 u_View;
uniform mat4 u_Projection;
uniform mat4 u_Transform;

out vec3 WorldPos;

void main()
{
    WorldPos = a_Position;
    gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);
}
