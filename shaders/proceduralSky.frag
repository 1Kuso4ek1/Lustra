// https://github.com/shff/opengl_sky

#version 460 core

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
    vec3 position = normalize(vertex) + vec3(0.0, 0.1, 0.0);

    if(position.y < -0.4)
        position.y = -0.4;

    // Atmosphere Scattering
    float mu = dot(normalize(position), normalize(sun));
    float rayleigh = 3.0 / (8.0 * 3.14) * (1.0 + mu * mu);
    vec3 mie = (Kr + Km * (1.0 - g * g) / (2.0 + g * g) / pow(1.0 + g * g - 2.0 * g * mu, 1.5)) / (Br + Bm);

    vec3 day_extinction = exp(-exp(-((position.y + sun.y * 4.0) * (exp(-position.y * 16.0) + 0.1) / 80.0) / Br) *
                         (exp(-position.y * 16.0) + 0.1) * Kr / Br) * exp(-position.y * exp(-position.y * 8.0 ) * 4.0) * exp(-position.y * 2.0);
                         
    vec3 night_extinction = vec3(1.0 - exp(sun.y)) * 0.05;
    vec3 extinction = mix(clamp(day_extinction, vec3(0.0), vec3(1.0)), night_extinction, -sun.y * 0.2 + 0.5);
    fragColor.rgb = rayleigh * mie * extinction;

    position -= vec3(0.0, 0.1, 0.0);

    // Cirrus Clouds
    if(cirrus > 0.0)
    {
        float density = smoothstep(1.0 - cirrus, 1.0, fbm(position.xyz / position.y * 2.0)) * 0.3;
        fragColor.rgb = mix(fragColor.rgb, extinction * 4.0, density * max(position.y, 0.0));
    }

    // Cumulus Clouds
    if(cumulus > 0.0)
    {
        for(int i = 0; i < 3; i++)
        {
            float density = smoothstep(1.0 - cumulus, 1.0, fbm((0.7 + float(i) * 0.01) * position.xyz / position.y));
            fragColor.rgb = mix(fragColor.rgb, extinction * density * 5.0, min(density, 1.0) * max(position.y, 0.0));
        }
    }

    fragColor.a = 1.0;
}

// Another possible implementation
// https://www.shadertoy.com/view/4tVSRt
/* const float coeiff = 0.25;
const vec3 totalSkyLight = vec3(0.3, 0.5, 1.0);

vec3 mie(float dist, vec3 sunL)
{
    return max(exp(-pow(dist, 0.25)) * sunL - 0.4, 0.0);
}

vec3 getSky(vec3 position, vec3 sunPosition)
{   
    float sunDistance = distance(position, sunPosition);
	
	float scatterMult = clamp(sunDistance, 0.0, 1.0);
	float sun = clamp(1.0 - smoothstep(0.01, 0.011, scatterMult), 0.0, 1.0);
	
	float dist = position.y;
	dist = (coeiff * mix(scatterMult, 1.0, dist)) / dist;
    
    vec3 mieScatter = mie(sunDistance, vec3(1.0));
	
	vec3 color = (dist * totalSkyLight);
    
    color = max(color, 0.0);
    
	color = max(mix(pow(color, 1.0 - color),
	                    color / (2.0 * color + 0.5 - color),
                        clamp(sunPosition.y * 2.0, 0.0, 1.0)), 0.0)
	                    + sun + mieScatter;
	
	color *= (pow(1.0 - scatterMult, 10.0) * 10.0) + 1.0;
	
	float underscatter = distance(sunPosition.y * 0.5 + 0.5, 1.0);
	
	color = mix(color, vec3(0.0), clamp(underscatter, 0.0, 1.0));
	
	return color;	
}

void main()
{
    fragColor = vec4(getSky(normalize(vertex), sun), 1.0);
} */
