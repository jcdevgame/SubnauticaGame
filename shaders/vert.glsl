#version 410 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 vertexColors;
layout(location = 2) in vec2 texCoords;
layout(location = 3) in vec3 normal;

uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_Projection;
uniform vec3 u_LightPos;
uniform mat4 u_LightSpaceMatrix;

out vec3 v_vertexColors;
out vec2 v_texCoords;
out vec3 v_vertexNormal;
out vec3 v_lightDirection;
out vec4 v_FragPosLightSpace;

void main()
{
    v_vertexColors = vertexColors;
    v_texCoords = texCoords;
    vec3 lightPos = u_LightPos;
    vec4 worldPosition = u_ModelMatrix * vec4(position, 1.0);
    v_vertexNormal = mat3(u_ModelMatrix) * normal;
    v_lightDirection = lightPos - worldPosition.xyz;

    v_FragPosLightSpace = u_LightSpaceMatrix * worldPosition;

    gl_Position = u_Projection * u_ViewMatrix * worldPosition;
}