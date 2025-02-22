#version 460 core

const float PI = 3.14159265359;

const float HammersleyPrecomputed[29] = float[]
(
    0, 0.5,	0.25, 0.75,	0.125, 0.625, 0.375, 0.875, 0.0625,	0.5625,	0.3125,
	0.8125,	0.1875,	0.6875,	0.4375,	0.9375,	0.03125, 0.53125, 0.28125, 0.78125,
	0.15625, 0.65625, 0.40625, 0.90625,	0.09375, 0.59375, 0.34375, 0.84375,	0.21875
);

uniform samplerCubeArray skybox;
uniform float roughness;
uniform int mip;

in vec3 vertex;

out vec4 fragColor;

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    
    return float(bits) * 2.3283064365386963e-10;
}

vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

// Precompute
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness * roughness;
	
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	
    vec3 H = vec3(
        cos(phi) * sinTheta,
        sin(phi) * sinTheta,
        cosTheta
    );
	
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    
    return normalize(sampleVec);
}

float DistributionGGX(float NdotH, float roughness)
{
    float denom = PI * pow(pow(NdotH, 2.0) * (pow(roughness, 4.0) - 1.0) + 1.0, 2.0);
    
    return pow(roughness, 4.0) / denom;
}

void main()
{
    vec3 N = normalize(vertex);
    vec3 R = N;
    vec3 V = R;

    float resolution = float(textureSize(skybox, 0).x);

    float totalWeight = 0.0;
    vec3 prefilteredColor = vec3(0.0);

    uint sampleCount = mip * 2 + 1;

    for(uint i = 0u; i < sampleCount; i++)
    {
        vec2 Xi = vec2(float(i) / float(sampleCount), HammersleyPrecomputed[i]);//Hammersley(i, sampleCount);
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if(NdotL <= 0.0)
            continue;

        float NdotH = dot(N, H);
        float HdotV = dot(H, V);

        float D = DistributionGGX(NdotH, roughness);
        float pdf = (D * NdotH / (4.0 * HdotV)) + 0.0001; 

        float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
        float saSample = 1.0 / (float(sampleCount) * pdf + 0.0001);

        float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

        prefilteredColor += textureLod(skybox, vec4(L, 0.0), mipLevel).rgb * NdotL;
        totalWeight += NdotL;
    }

    prefilteredColor = prefilteredColor / totalWeight;

    fragColor = vec4(max(prefilteredColor, 0.0), 1.0);
}
