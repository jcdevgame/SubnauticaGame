#version 410 core
#extension GL_ARB_bindless_texture : require

in vec3 v_vertexColors;
in vec2 v_texCoords;
in vec3 v_vertexNormal;
in vec3 v_lightDirection;
in vec4 v_FragPosLightSpace; // Position of the fragment in light space

out vec4 color;

uniform sampler2D shadowMap; // The shadow map
uniform sampler2D u_TextureHandle; // Bindless texture handle

uniform vec3 u_LightColor;

void main()
{ 
    //vec3 lightColor = vec3(0.960, 0.895, 0.605);
    vec3 lightColor = u_LightColor;
    vec3 ambientColor = vec3(0.2, 0.2, 0.2); // Ambient light to soften shadows
    vec3 normalVector = normalize(v_vertexNormal);
    vec3 lightVector = normalize(v_lightDirection);
    float dotProduct = dot(normalVector, lightVector);
    float brightness = max(dotProduct, 0.0);
    vec3 diffuse = brightness * lightColor;

    // Calculate the shadow factor
    vec3 projCoords = v_FragPosLightSpace.xyz / v_FragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // Transform to [0,1] range
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    float bias = 0.005; // Add a small bias to avoid shadow acne
    float shadow = currentDepth - bias > closestDepth  ? 0.5 : 1.0; // If the current fragment is in shadow, reduce its brightness

    vec3 finalColor = (ambientColor + shadow * diffuse);
    color = texture(u_TextureHandle, v_texCoords) * vec4(finalColor, 1.0);
}