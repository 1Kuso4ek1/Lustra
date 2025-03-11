// MADE BY DEEPSEEK JUST FOR TEST :)

#version 460 core

uniform int albedoType;
uniform int normalType;
uniform int metallicType;
uniform int roughnessType;
uniform int aoType;
uniform int emissionType;

uniform sampler2D albedoTexture;
uniform sampler2D normalTexture;
uniform sampler2D metallicTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D aoTexture;
uniform sampler2D emissionTexture;

uniform vec4 albedoValue;
uniform float metallicValue;
uniform float roughnessValue;
uniform vec3 emissionValue;
uniform float emissionStrength;

uniform float time;

in vec3 mPosition;
in vec3 mNormal;
in mat3 TBN;
in vec2 coord;

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gAlbedo;
layout(location = 2) out vec4 gNormal;
layout(location = 3) out vec4 gCombined;
layout(location = 4) out vec4 gEmission;

float checkerPattern(vec2 uv, float scale)
{
    vec2 st = uv * scale;
    vec2 ipos = floor(st);
    return mod(ipos.x + ipos.y, 2.0) < 1.0 ? 0.0 : 1.0;
}

float random(vec2 st)
{
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

void main()
{
    float metallic = metallicValue;
    float roughness = roughnessValue;
    float ao = 1.0;

    gPosition = vec4(mPosition, 1.0);
    
    vec2 wrappedCoord = fract(coord);
    if(albedoType == 0)
	{
        float pattern = checkerPattern(wrappedCoord, 8.0);
        vec3 baseColor = albedoValue.rgb;
        vec3 altColor = baseColor * vec3(0.8, 1.2, 0.8);
        gAlbedo = vec4(mix(baseColor, altColor, pattern), albedoValue.a);
    }
    else
        gAlbedo = texture(albedoTexture, wrappedCoord);

    if(normalType == 0)
	{
        vec3 distortedNormal = mNormal;
        float waveIntensity = 0.05;        
        
        distortedNormal.x += sin(abs(mPosition.z) * 2.0 + time * 2.0) * waveIntensity;
        distortedNormal.z += cos(abs(mPosition.x) * 2.0 + time * 2.0) * waveIntensity;
        
        float len = length(distortedNormal);
        if(len > 0.0)
            distortedNormal /= len;
			
        gNormal = vec4(distortedNormal, 1.0);
    }
    else
	{
        vec3 normalMap = texture(normalTexture, wrappedCoord).xyz * 2.0 - 1.0;
        gNormal = vec4(normalize(TBN * normalize(normalMap)), 1.0);
    }
    
    if(metallicType == 1)
        metallic = texture(metallicTexture, wrappedCoord).b;
    else
	{
        float distFromCenter = length(mPosition.xz);
        metallic = clamp(metallicValue * (0.5 + 0.5 * sin(distFromCenter * 4.0 - time)), 0.0, 1.0);
    }

    if(roughnessType == 1)
        roughness = texture(roughnessTexture, coord).g;
    if(aoType == 1)
        ao = texture(aoTexture, coord).r;
    
    if(emissionType == 0)
	{
        float cellsDensity = 3.0;
        float sparkSize = 0.4;
        float sparkSharpness = 8.0;
        
        vec2 gridCoord = coord * cellsDensity;
        vec2 cellPos = fract(gridCoord);
        vec2 cellID = floor(gridCoord);

        float t = fract(time * 0.2);
        float seed = random(cellID + floor(t));
        float sparkLife = fract(t + seed * 1.5);

        vec2 sparkCenter = vec2(0.5);

        vec2 offset = cellPos - sparkCenter;
        float dist = length(offset);
        float radius = sparkSize * (0.8 + 0.2 * sin(seed * 6.283));
        
        float spark = smoothstep(radius + 0.1, radius - 0.1, dist) * 
                     pow(smoothstep(1.0, 0.0, sparkLife), sparkSharpness);

        spark *= step(sparkLife, 0.95);

        vec3 glowingEmission = emissionValue * emissionStrength * (/* pulse +  */spark * 1.0);
        gEmission = vec4(glowingEmission, 1.0);
    }
    else
        gEmission = vec4(texture(emissionTexture, wrappedCoord).rgb * emissionStrength, 1.0);

    gCombined = vec4(metallic, roughness, ao, 1.0);
}
