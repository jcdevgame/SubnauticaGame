#version 410 core

in vec3 v_vertexColors;
in vec2 v_texCoords;
in vec3 v_vertexNormal;
in vec3 v_lightDirection;
in vec4 v_FragPosLightSpace;

out vec4 color;

uniform sampler2D shadowMap;
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

    vec3 projCoords = v_FragPosLightSpace.xyz / v_FragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth ? 0.5 : 1.0;

    vec3 finalColor = (ambientColor + shadow * diffuse);
    vec3 coords = vec3(v_texCoords, float(u_TextureArrayIndex));

    color = texture(textureArray, coords) * vec4(finalColor, 1.0);
    
    // Debugging output
    /*
    if (u_TextureArrayIndex == 0) {
        color = vec4(1.0, 0.0, 0.0, 1.0); // Red for index 0
    } else if (u_TextureArrayIndex == 1) {
        color = vec4(0.0, 1.0, 0.0, 1.0); // Green for index 1
    } else {
        color = vec4(0.0, 0.0, 1.0, 1.0); // Blue for other indices
    }
    */
}
