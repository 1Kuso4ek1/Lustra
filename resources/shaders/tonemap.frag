#version 460 core

uniform sampler2D frame;
uniform sampler2D bloom;
uniform sampler2D ssr;
uniform sampler2D gAlbedo;
uniform sampler2D gCombined;
uniform sampler2D lut;

in vec2 coord;

out vec4 fragColor;

uniform int algorithm;
uniform float exposure;
uniform float bloomStrength;

uniform vec3 colorGrading;

uniform float colorGradingIntensity;
uniform float vignetteIntensity;
uniform float vignetteRoundness;
uniform float filmGrain;
uniform float contrast;
uniform float saturation;
uniform float brightness;

uniform float time;

mat3 acesIn = mat3(0.59719, 0.07600, 0.02840,
                   0.35458, 0.90834, 0.13383,
                   0.04823, 0.01566, 0.83777);

mat3 acesOut = mat3(1.60475, -0.10208, -0.00327,
                    -0.53108,  1.10813, -0.07276,
                    -0.07367, -0.00605, 1.07602);

float Hash(vec2 vec)
{
    vec3 v3 = fract(vec3(vec.xyx) * 0.1031);
         v3 += dot(v3, v3.yzx + 33.33);

    return fract((v3.x + v3.y) * v3.z);
}

vec3 ACES(vec3 color)
{
    vec3 v = acesIn * color;
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;

    return acesOut * (a / b);
}

vec3 ACESFilm(vec3 color)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;

    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

vec3 Reinhard(vec3 color)
{
    return color / (color + 1.0);
}

vec3 Uncharted2Helper(vec3 color)
{
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;

    return ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
}

vec3 Uncharted2(vec3 color)
{
    const float W = 11.2;
    const float exposureBias = 2.0;
    
    vec3 curr = Uncharted2Helper(color * exposureBias);
    vec3 whiteScale = 1.0 / Uncharted2Helper(vec3(W));
    
    return curr * whiteScale;
}

vec3 Filmic(vec3 color)
{
    vec3 X = max(vec3(0.0), color - 0.004);
    vec3 result = (X * (6.2 * X + 0.5)) / (X * (6.2 * X + 1.7) + 0.06);

    return pow(result, vec3(2.2));
}

vec3 Lottes(vec3 color)
{
    const float a = 1.6;
    const float d = 0.977;
    const float hdrMax = 8.0;
    const float midIn = 0.18;
    const float midOut = 0.267;

    float l = dot(color, vec3(0.2126, 0.7152, 0.0722));

    float lm = l / hdrMax;
    float ls = lm > 0.0 ? midOut / midIn * pow(lm, a - 1.0) : 0.0;
    float lmSq = pow(lm, a);
    float ltm = (lmSq * midOut) / (lmSq + midOut);
    float lFinal = ltm * d;

    return color * (lFinal / l);
}

vec3 ApplyTonemap(vec3 color)
{
    switch(algorithm)
    {
        case 0: return ACES(color);
        case 1: return ACESFilm(color);
        case 2: return Reinhard(color);
        case 3: return Uncharted2(color);
        case 4: return Filmic(color);
        case 5: return Lottes(color);

        default: return color;
    }
}

vec3 ApplySSR(vec3 color, vec2 texelSize)
{
    if(length(textureLod(ssr, coord, 5.0).rgb) > 0.0)
    {
        vec3 combined = texture(gCombined, coord).rgb;

        float lod = 8.0 * combined.y;

        vec4 ssrSample = textureLod(ssr, coord + sqrt(lod) * texelSize, lod);

        if(combined.x != 0.0 && combined.y < 0.9/*  && dot(ssrSample, vec3(0.2126, 0.7152, 0.0722)) > 0.1 */ /* && ssrSample.a > 0.7 */)
        {
            vec3 albedo = texture(gAlbedo, coord).rgb;

            vec3 f0 = mix(vec3(0.04), albedo, combined.x);

            float mixFactor = combined.x * (1.0 - combined.y);

            float alphaMask = smoothstep(0.1, 0.9, ssrSample.a);
            
            vec2 edge = abs(coord * 2.0 - 1.0);
            float edgeFade = 1.0 - smoothstep(0.7, 1.0, max(edge.x, edge.y));
            
            color = mix(color, f0 * ssrSample.rgb, mixFactor * alphaMask * edgeFade);
        }
    }

    return color;
}

vec3 ApplyColorGrading(vec3 color)
{
    float lumaA = dot(color, vec3(0.2126, 0.7152, 0.0722));
    float lumaB = dot(colorGrading, vec3(0.2126, 0.7152, 0.0722));
    
    vec3 mixed = mix(color, colorGrading, colorGradingIntensity);

    color = mixed * (mix(lumaA, lumaB, colorGradingIntensity) / (dot(mixed, vec3(0.2126, 0.7152, 0.0722)) + 1e-5));

    color *= brightness;

    color = (color - 0.5) * contrast + 0.5;

    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));

    return mix(vec3(luminance), color, saturation);
}

vec3 ApplyLUT(vec3 color)
{
    if(texture(lut, vec2(0.1)).r == 0.0)
        return color;

    color = clamp(color, 0.0, 1.0);

    vec2 lutTextureSize = textureSize(lut, 0);
    vec4 lutSize = vec4(lutTextureSize, 1.0 / lutTextureSize);

    vec3 scaledColor = color * (lutSize.y - 1.0);
    float bFrac = fract(scaledColor.b);

    vec2 texc = (0.5 + scaledColor.rg) * lutSize.zw;

    texc.x += (scaledColor.b - bFrac) * lutSize.w;

    vec3 color0 = texture(lut, texc).rgb;
    vec3 color1 = texture(lut, vec2(texc.x + lutSize.w, texc.y)).rgb;

    // Weird fix of yellow dots on the color area edges
    if(dot(color0, vec3(0.7, 0.7, 0.1)) > 0.5)
        return color;

    return mix(color0, color1, bFrac);
}

vec3 ApplyVignette(vec3 color)
{
    vec2 uv = (coord - 0.5) * 2.0;

    float dist = length(uv);
    float vignette = 1.0 - pow(dist * vignetteRoundness, 1.0 - vignetteIntensity);

    return color * vignette;
}

vec3 ApplyFilmGrain(vec3 color)
{
    return color + (Hash(coord * vec2(312.24, 1030.057) * (time + 1.0)) * 2.0 - 1.0) * filmGrain;
}

void main()
{
    vec2 texelSize = 1.0 / textureSize(frame, 0);

    vec4 color = texture(frame, coord);
    vec4 bloomColor = texture(bloom, coord);

    color.rgb += bloomColor.rgb * bloomStrength;
    
    color.rgb = ApplySSR(color.rgb, texelSize);
    color.rgb = ApplyTonemap(color.rgb * exposure);
    color.rgb = ApplyLUT(color.rgb);
    color.rgb = ApplyFilmGrain(color.rgb);
    color.rgb = ApplyVignette(color.rgb);
    color.rgb = ApplyColorGrading(color.rgb);

    color.rgb = pow(color.rgb, vec3(1.0 / 2.2));

    fragColor = color;
}
