#version 460 core

const float PI = 3.14159265359;

const float maxReflectionLod = 8.0;

const int maxLights = 128;
const int maxShadows = 4;

const vec3 F0 = vec3(0.04);

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

uniform samplerCubeArray irradiance;
uniform samplerCubeArray prefiltered;

uniform sampler2D gPosition;
uniform sampler2D gAlbedo;
uniform sampler2D gNormal;
uniform sampler2D gCombined;
uniform sampler2D gEmission;

uniform sampler2D brdf;

in vec2 coord;

out vec4 fragColor;

float ambientStrength = 0.1;
float specularStrength = 5.0;
float shininess = 256.0;

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
	
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);

    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	
    return ggx1 * ggx2;
}

float CalculateShadow(int index, vec4 position)
{
    vec4 lightSpacePosition = shadows[index].lightSpaceMatrix * position;

    vec3 projCoords = lightSpacePosition.xyz / lightSpacePosition.w;
    projCoords = projCoords * vec3(0.5, -0.5, 0.5) + 0.5;

    if(projCoords.z > 1.0)
        return 0.0;
    
    projCoords.z -= shadows[index].bias;

    return 1.0 - texture(shadowMaps[index], projCoords);
}

float CalculateShadows(vec4 position)
{
    float ret = 0.0;

    for(int i = 0; i < numShadows; i++)
    {
        float tmp = CalculateShadow(i, position);
        
        if(ret < tmp)
            ret = tmp;
    }

    return ret;
}

vec3 CalculateLight(int index, vec3 worldPosition, vec3 V, vec3 N, vec3 albedo, float metallic, float roughness)
{
    float theta = dot(normalize(lights[index].position - worldPosition), normalize(-lights[index].direction));
    float intensity = 1.0;
    
    if(lights[index].cutoff != 1.0)
        intensity = clamp((theta - lights[index].outerCutoff) / (lights[index].cutoff - lights[index].outerCutoff), 0.0, 1.0);
    
    if(theta < lights[index].cutoff && intensity <= 0.0)
        return vec3(0.0);

    vec3 L = normalize(lights[index].position - worldPosition);
    vec3 H = normalize(V + L);
    vec3 F = FresnelSchlickRoughness(max(dot(H, V), 0.0), mix(F0, albedo, metallic), roughness);

    float dist = length(lights[index].position - worldPosition);
    float attenuation = 1.0 / (dist * dist);

    vec3 radiance = lights[index].color * lights[index].intensity/*  * attenuation */;

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.0001;

    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metallic);
    
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

vec3 CalculateLights(vec3 worldPosition, vec3 V, vec3 N, vec3 albedo, float metallic, float roughness)
{
    vec3 totalLighting = vec3(0.0);
    for(int i = 0; i < numLights; i++)
    {
        totalLighting += CalculateLight(i, worldPosition, V, N, albedo, metallic, roughness);
    }

    return totalLighting;
}

void main()
{
    vec4 posSample = texture(gPosition, coord);
    if(posSample.w <= 0.001)
        discard;

    vec3 worldPosition = posSample.xyz;
    vec3 albedo = /* pow( */texture(gAlbedo, coord).rgb/* , vec3(2.2)) */; // ???
    vec3 normal = normalize(texture(gNormal, coord).xyz);
    vec3 combined = texture(gCombined, coord).rgb;
    vec3 emission = texture(gEmission, coord).rgb;

    float metallic = combined.r;
    float roughness = combined.g;
    float ao = combined.b;

    vec3 V = normalize(cameraPosition - worldPosition);
    vec3 R = reflect(vec3(-V.x, -V.y, V.z), normal);

    float NdotV = clamp(dot(normal, V), 0.0, 0.99);

    vec3 lighting = CalculateLights(worldPosition, V, normal, albedo, metallic, roughness);

    float shadow = CalculateShadows(vec4(worldPosition, 1.0));

    vec3 F = FresnelSchlickRoughness(NdotV, mix(F0, albedo, metallic), roughness);

    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metallic);
    
    vec3 irradiance = texture(irradiance, vec4(normal, 0.0)).rgb;
    vec3 diffuse = irradiance * albedo;
    
    vec3 prefilteredColor = textureLod(prefiltered, vec4(R, 0.0), roughness * maxReflectionLod).rgb;
    vec2 envBRDF = texture(brdf, vec2(NdotV, roughness)).rg;
    vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
    
    vec3 ambient = (kD * diffuse + specular) * ao;

    vec3 finalColor = (ambient + lighting) * (1.0 - shadow) + emission + (ambient / 5.0) * shadow;

    fragColor = vec4(finalColor, 1.0);
}
