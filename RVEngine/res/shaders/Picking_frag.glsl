#version 460 core

layout(location = 0) out uvec3 EntityColor;

uniform uint u_ObjectIndex;

void main()
{
    EntityColor = uvec3(u_ObjectIndex, 0, gl_PrimitiveID);
}