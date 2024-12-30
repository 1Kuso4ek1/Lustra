// https://github.com/shff/opengl_sky

#version 450 core

uniform sampler2D cubemap;

in vec3 vertex;
in vec3 sun;

out vec4 fragColor;

uniform float cirrus = 0.3;
uniform float cumulus = 0.6;

const float Br = 0.0025;
const float Bm = 0.0003;
const float g =  0.9800;
const vec3 nitrogen = vec3(0.650, 0.570, 0.475);
const vec3 Kr = Br / pow(nitrogen, vec3(4.0));
const vec3 Km = Bm / pow(nitrogen, vec3(0.84));

float hash(float n)
{
    return fract(sin(n) * 43758.5453123);
}

float noise(vec3 x)
{
    vec3 f = fract(x);
    float n = dot(floor(x), vec3(1.0, 157.0, 113.0));
    return mix( mix(mix(hash(n +   0.0), hash(n +   1.0), f.x),
                    mix(hash(n + 157.0), hash(n + 158.0), f.x), f.y),
                mix(mix(hash(n + 113.0), hash(n + 114.0), f.x),
                    mix(hash(n + 270.0), hash(n + 271.0), f.x), f.y), f.z);
}

const mat3 m = mat3(0.0, 1.60,  1.20, -1.6, 0.72, -0.96, -1.2, -0.96, 1.28);

float fbm(vec3 p)
{
    float f = 0.0;
    
    f += noise(p) / 2; p = m * p * 1.1;
    f += noise(p) / 4; p = m * p * 1.2;
    f += noise(p) / 6; p = m * p * 1.3;
    f += noise(p) / 12; p = m * p * 1.4;
    f += noise(p) / 24;

    return f;
}

void main()
{
    vec3 position = normalize(vertex);

    if(position.y < -0.4)
        position.y = -0.4;

    // Atmosphere Scattering
    float mu = dot(normalize(position), normalize(sun));
    float rayleigh = 3.0 / (8.0 * 3.14) * (1.0 + mu * mu);
    vec3 mie = (Kr + Km * (1.0 - g * g) / (2.0 + g * g) / pow(1.0 + g * g - 2.0 * g * mu, 1.5)) / (Br + Bm);

    vec3 day_extinction = exp(-exp(-((position.y + sun.y * 4.0) * (exp(-position.y * 16.0) + 0.1) / 80.0) / Br) * (exp(-position.y * 16.0) + 0.1) * Kr / Br) * exp(-position.y * exp(-position.y * 8.0 ) * 4.0) * exp(-position.y * 2.0) * 4.0;
    vec3 night_extinction = vec3(1.0 - exp(sun.y)) * 0.2;
    vec3 extinction = mix(day_extinction, night_extinction, -sun.y * 0.2 + 0.5);
    fragColor.rgb = rayleigh * mie * extinction;

    // Cirrus Clouds
    float density = smoothstep(1.0 - cirrus, 1.0, fbm(position.xyz / position.y * 2.0)) * 0.3;
    fragColor.rgb = mix(fragColor.rgb, extinction * 4.0, density * max(position.y, 0.0));

    // Cumulus Clouds
    for (int i = 0; i < 3; i++)
    {
        float density = smoothstep(1.0 - cumulus, 1.0, fbm((0.7 + float(i) * 0.01) * position.xyz / position.y));
        fragColor.rgb = mix(fragColor.rgb, extinction * density * 5.0, min(density, 1.0) * max(position.y, 0.0));
    }

    // Dithering Noise
    fragColor.rgb += noise(position * 1000) * 0.01;
}