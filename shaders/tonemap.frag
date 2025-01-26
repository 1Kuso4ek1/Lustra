#version 460 core

uniform sampler2D frame;
uniform sampler2D bloom;

in vec2 coord;

out vec4 fragColor;

uniform int algorithm;
uniform float exposure;
uniform float bloomStrength;

mat3 acesIn = mat3(0.59719, 0.07600, 0.02840,
                   0.35458, 0.90834, 0.13383,
                   0.04823, 0.01566, 0.83777);

mat3 acesOut = mat3(1.60475, -0.10208, -0.00327,
                    -0.53108,  1.10813, -0.07276,
                    -0.07367, -0.00605, 1.07602);

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

vec3 Apply(vec3 color)
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

void main()
{
    vec4 color = texture(frame, coord);
    vec4 bloomColor = texture(bloom, coord);
    
    color.rgb += bloomColor.rgb * bloomStrength;
    
    color.rgb = Apply(color.rgb * exposure);
    color.rgb = pow(color.rgb, vec3(1.0 / 2.2));

    fragColor = color;
}
