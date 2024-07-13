#version 330 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 Texcoord;

// Transformation matrices.
uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 normalMatrix;
uniform mat4 MVP;

// Data pass to fragment shader.
out vec3 iPosition;
out vec3 iNormal;
out vec2 iTexCoord;


void main()
{
    gl_Position = MVP * vec4(Position, 1.0);

    vec4 tmpPos = viewMatrix * worldMatrix * vec4(Position, 1.0);
    iPosition = tmpPos.xyz / tmpPos.w;

    iNormal = normalize((normalMatrix * vec4(Normal, 0.0)).xyz);

    iTexCoord = Texcoord;
}