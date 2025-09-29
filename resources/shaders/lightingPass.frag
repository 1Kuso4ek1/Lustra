#version 460 core

#define LAMBERT_DIFFUSE 0
#define OREN_NAYAR_DIFFUSE 1
#define BURLEY_DIFFUSE 2

#define DIFFUSE_METHOD BURLEY_DIFFUSE

const float PI = 3.14159265359;

const float maxReflectionLod = 8.0;

const int maxLights = 128;
const int maxShadows = 4;

const vec3 F0 = vec3(0.04);

struct Light
{
    int shadow;

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

uniform mat4 view;

uniform samplerCubeArray irradiance;
uniform samplerCubeArray prefiltered;

uniform sampler2D gPosition;
uniform sampler2D gAlbedo;
uniform sampler2D gNormal;
uniform sampler2D gCombined;
uniform sampler2D gEmission;

uniform sampler2D gtao;
uniform sampler2D brdf;

in vec2 coord;

out vec4 fragColor;

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float FresnelSchlick(float f0, float f90, float VdotH)
{
    return f0 + (f90 - f0) * pow(1.0 - VdotH, 5.0);
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

// https://github.com/glslify/glsl-diffuse-oren-nayar
float OrenNayarDiffuse(
    float LdotV,
    float NdotV,
    float NdotL,
    float roughness,
    float albedo
) {
    float s = LdotV - NdotL * NdotV;
    float t = mix(1.0, max(max(NdotL, NdotV), 0.001), step(0.0, s));

    float sigma2 = pow(roughness, 2.0);
    float A = 1.0 + sigma2 * (albedo / (sigma2 + 0.13) + 0.5 / (sigma2 + 0.33));
    float B = 0.45 * sigma2 / (sigma2 + 0.09);

    return albedo * max(0.0, NdotL) * (A + B * s / t) / PI;
}

// https://github.com/google/filament/blob/436dffcbb39f8fa8741b8cc68cdd5776f2a98901/shaders/src/surface_brdf.fs#L241
float BurleyDiffuse(float NdotV, float NdotL, float LdotH, float roughness)
{
    // Burley 2012, "Physically-Based Shading at Disney"
    float f90 = 0.5 + 2.0 * roughness * pow(LdotH, 2.0);
    float lightScatter = FresnelSchlick(1.0, f90, NdotL);
    float viewScatter  = FresnelSchlick(1.0, f90, NdotV);

    return lightScatter * viewScatter * (1.0 / PI);
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
    float attenuation = 1.0 / clamp(dist * 0.1, 0.0, 100.0); // Need a more flexible method

    vec3 radiance = lights[index].color * lights[index].intensity * intensity * attenuation;

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.0001;

    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metallic);

#if DIFFUSE_METHOD == BURLEY_DIFFUSE
    float LdotH = max(dot(L, H), 0.0);

    float burley = BurleyDiffuse(NdotV, NdotL, LdotH, roughness);

    return (kD * albedo * burley + specular) * radiance * NdotL;
#elif DIFFUSE_METHOD == OREN_NAYAR_DIFFUSE
    float LdotV = max(dot(L, V), 0.0);

    vec3 orenNayar = vec3(
        OrenNayarDiffuse(LdotV, NdotV, NdotL, roughness, albedo.r),
        OrenNayarDiffuse(LdotV, NdotV, NdotL, roughness, albedo.g),
        OrenNayarDiffuse(LdotV, NdotV, NdotL, roughness, albedo.b)
    );

    return (kD * orenNayar + specular) * radiance;
#elif DIFFUSE_METHOD == LAMBERT_DIFFUSE
    return (kD * albedo / PI + specular) * radiance * NdotL;
#endif
}

void CalculateLights(
    inout vec3 totalLighting, inout vec3 totalNoShadow,

    vec3 worldPosition, vec3 V, vec3 N, vec3 albedo, float metallic, float roughness
)
{
    for(int i = 0; i < numLights; i++)
    {
        vec3 res = CalculateLight(i, worldPosition, V, N, albedo, metallic, roughness);

        if(lights[i].shadow == 1)
            totalLighting += res;
        else
            totalNoShadow += res;
    }
}

void main()
{
    vec4 posSample = inverse(view) * texture(gPosition, coord);
    if(posSample.w <= 0.001)
        discard;

    vec3 worldPosition = posSample.xyz;
    vec3 albedo = texture(gAlbedo, coord).rgb;
    vec3 normal = normalize(texture(gNormal, coord).xyz);
    vec3 combined = texture(gCombined, coord).rgb;
    vec3 emission = texture(gEmission, coord).rgb;

    float metallic = combined.r;
    float roughness = combined.g;
    float ao = combined.b * clamp(1.0 - texture(gtao, coord).r, 0.0, 1.0);

    vec3 V = normalize(cameraPosition - worldPosition);
    vec3 R = reflect(-V, normal) * vec3(1.0, 1.0, -1.0);

    float NdotV = clamp(dot(normal, V), 0.0, 0.99);

    vec3 totalLighting = vec3(0.0);
    vec3 totalNoShadow = vec3(0.0);

    CalculateLights(totalLighting, totalNoShadow, worldPosition, V, normal, albedo, metallic, roughness);

    float shadow = CalculateShadows(vec4(worldPosition, 1.0));

    vec3 F = FresnelSchlickRoughness(NdotV, mix(F0, albedo, metallic), roughness);

    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metallic);

    vec3 irradiance = texture(irradiance, vec4(normal, 0.0)).rgb;
    vec3 diffuse = irradiance * albedo;

    vec3 prefilteredColor = textureLod(prefiltered, vec4(R, 0.0), roughness * maxReflectionLod).rgb;
    vec2 envBRDF = texture(brdf, vec2(NdotV, roughness)).rg;
    vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    vec3 ambient = (kD * diffuse + specular);

    vec3 finalColor = (((ambient + totalLighting) * (1.0 - shadow)) + totalNoShadow + emission + (ambient / 5.0) * shadow) * ao;

    fragColor = vec4(finalColor, 1.0);
}
