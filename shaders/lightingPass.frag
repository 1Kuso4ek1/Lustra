#version 460 core

const int maxLights = 128;
const int maxShadows = 4;

struct Light
{
    vec3 position;
    vec3 direction;
    vec3 color;
    
    float intensity, cutoff, outerCutoff;
};

struct Shadow
{
    mat4 lightSpaceMatrix;

    float bias;
};

layout(std140) uniform lightBuffer
{
    Light lights[maxLights];
};

layout(std140) uniform shadowBuffer
{
    Shadow shadows[maxShadows];
};

uniform sampler2DShadow shadowMaps[maxShadows];

uniform vec3 cameraPosition;

uniform int numLights;
uniform int numShadows;

uniform sampler2D gPosition;
uniform sampler2D gAlbedo;
uniform sampler2D gNormal;
uniform sampler2D gCombined;
uniform sampler2D gEmission;

in vec2 coord;

out vec4 fragColor;

float ambientStrength = 0.1;
float specularStrength = 5.0;
float shininess = 256.0;

float CalculateShadows(vec4 position)
{
    float ret = 0.0;

    for(int i = 0; i < numShadows; i++)
    {
        vec4 lightSpacePosition = shadows[i].lightSpaceMatrix * position;

        vec3 projCoords = lightSpacePosition.xyz / lightSpacePosition.w;
        projCoords = projCoords * vec3(0.5, -0.5, 0.5) + 0.5;

        if(projCoords.z > 1.0)
            continue;
        
        projCoords.z -= shadows[i].bias;

        // float tmp = 1.0 - textureProj(shadowMaps[i], lightSpacePosition, shadows[i].bias);
        float tmp = 1.0 - texture(shadowMaps[i], projCoords);
        
        if(ret < tmp)
            ret = tmp;
    }

    return ret;
}

void main()
{
    vec4 posSample = texture(gPosition, coord);
    if(posSample.w <= 0.001)
        discard;

    vec3 worldPosition = posSample.xyz;
    vec3 albedo = texture(gAlbedo, coord).rgb;
    vec3 normal = normalize(texture(gNormal, coord).xyz);

    vec3 totalAmbient = vec3(0.0);
    vec3 totalDiffuse = vec3(0.0);
    vec3 totalSpecular = vec3(0.0);

    vec3 viewDir = normalize(cameraPosition - worldPosition);

    for(int i = 0; i < numLights; i++)
    {
        float theta = dot(normalize(lights[i].position - worldPosition), normalize(-lights[i].direction));
        float intensityMultiplier = 1.0;

        if(lights[i].cutoff != 1.0)
            intensityMultiplier = clamp((theta - lights[i].outerCutoff) / (lights[i].cutoff - lights[i].outerCutoff), 0.0, 1.0);
        if(theta < lights[i].cutoff && intensityMultiplier <= 0.0)
            intensityMultiplier = 0.0;

        vec3 lightDir = normalize(lights[i].position - worldPosition);
        vec3 halfDir = normalize(lightDir + viewDir);

        vec3 diffuse = max(dot(normal, lightDir), 0.0) * lights[i].color * lights[i].intensity * intensityMultiplier;
        totalDiffuse += diffuse;

        vec3 ambient = ambientStrength * lights[i].color;
        totalAmbient += ambient;
        
        float spec = pow(max(dot(normal, halfDir), 0.0), shininess);
        totalSpecular += spec * lights[i].color * lights[i].intensity * specularStrength;
    }

    float shadow = CalculateShadows(vec4(worldPosition, 1.0));

    vec3 finalColor = (totalAmbient + (1.0 - shadow) * (totalDiffuse + totalSpecular)) * albedo;

    fragColor = vec4(finalColor, 1.0);
}
