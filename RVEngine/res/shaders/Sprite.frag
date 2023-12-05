#version 460 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out uvec3 EntityColor;

uniform vec4 u_Color;
uniform uint u_ObjectIndex;
in vec2 TexCoords;

layout(binding=0) uniform sampler2D u_Albedo;


void main()
{
    vec4 textureColor = texture(u_Albedo, TexCoords).rgba;

    vec4 finalColor = textureColor * u_Color;

    finalColor = clamp(finalColor, 0.0, 1.0);

    FragColor = finalColor;
    EntityColor = uvec3(u_ObjectIndex, 0, gl_PrimitiveID);

}