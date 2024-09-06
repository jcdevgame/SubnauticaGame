#version 410 core

in vec3 v_vertexColors;
in vec2 v_texCoords;
in vec3 v_vertexNormal;
in vec3 v_lightDirection;

out vec4 color;

uniform sampler2DArray textureArray;
uniform vec3 u_LightColor;
uniform int u_TextureArrayIndex;

void main()
{ 
    vec3 lightColor = u_LightColor;
    vec3 ambientColor = vec3(0.2, 0.2, 0.2);
    vec3 normalVector = normalize(v_vertexNormal);
    vec3 lightVector = normalize(v_lightDirection);
    float dotProduct = dot(normalVector, lightVector);
    float brightness = max(dotProduct, 0.0);
    vec3 diffuse = brightness * lightColor;

    vec3 finalColor = ambientColor + diffuse;
    vec3 coords = vec3(v_texCoords, float(u_TextureArrayIndex));

    color = texture(textureArray, coords) * vec4(finalColor, 1.0);
}