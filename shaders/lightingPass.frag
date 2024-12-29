#version 450 core

const int maxLights = 128;

struct Light
{
    vec3 position;
    vec3 direction;
    vec3 color;
    
    float intensity, cutoff, outerCutoff;
};

layout(std140) uniform lightBuffer
{
    Light lights[maxLights];
};

uniform vec3 cameraPosition;

uniform int numLights;

uniform sampler2D gPosition;
uniform sampler2D gAlbedo;
uniform sampler2D gNormal;

in vec2 coord;

out vec4 fragColor;

float ambientStrength = 0.1;
float specularStrength = 5.0;
float shininess = 256.0;

void main()
{
    vec4 posSample = texture(gPosition, coord);
    if(posSample.w <= 0.001f)
        discard;

    vec3 worldPosition = posSample.xyz;
    vec3 albedo = texture(gAlbedo, coord).rgb;
    vec3 normal = normalize(texture(gNormal, coord).xyz);

    vec3 totalDiffuse = vec3(0.0);
    vec3 totalAmbient = vec3(0.0);
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

    vec3 finalColor = (totalAmbient + totalDiffuse + totalSpecular) * albedo;

    fragColor = vec4(finalColor, 1.0);
}
