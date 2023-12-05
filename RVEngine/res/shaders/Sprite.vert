#version 460 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;
uniform bool u_Billboard;
out vec2 TexCoords;

void main()
{
    TexCoords = a_TexCoord;

    if (u_Billboard)
    {
        mat3 viewMatrix3x3 = mat3(inverse(mat4(u_ViewProjection)));

        vec3 right = normalize(viewMatrix3x3[0]);
        vec3 up = normalize(viewMatrix3x3[1]);

        vec3 finalPosition = a_Position.x * right + a_Position.y * up + a_Position.z * viewMatrix3x3[2];

        gl_Position = u_ViewProjection * u_Transform * vec4(finalPosition, 1.0);
    } else
    {
        gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
    }

}
