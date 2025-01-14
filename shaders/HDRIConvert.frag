#version 460 core

const vec2 invatan = vec2(0.1591, 0.3183);

uniform sampler2D hdri;

in vec3 vertex;

out vec4 fragColor;

vec2 SampleMap(vec3 vec)
{
    vec2 uv = -vec2(atan(vec.z, vec.x), asin(vec.y));
    uv *= invatan;
    uv += 0.5;

    return uv;
}

void main()
{
    vec2 coord = SampleMap(normalize(vertex)); 
    fragColor = vec4(clamp(texture(hdri, coord).rgb, vec3(0.0), vec3(500.0)), 1.0);
}

