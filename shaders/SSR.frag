#version 460 core

layout(std140) uniform matrices
{
    mat4 model, view, projection;
};

uniform sampler2D gNormal;
uniform sampler2D gCombined;
uniform sampler2D depth;
uniform sampler2D frame;

uniform int maxSteps = 50;
uniform int maxBinarySearchSteps = 10;

uniform float rayStep = 0.01;

in vec2 coord;

out vec4 fragColor;

const float depthBias = 0.5;

vec2 uv;

vec3 ViewPosFromDepth(vec2 uv)
{
    vec4 clipPos = vec4(uv * 2.0 - 1.0, textureLod(depth, uv, 4.0).r * 2.0 - 1.0, 1.0);
    vec4 viewPos = inverse(projection) * clipPos;

    return viewPos.xyz / viewPos.w;
}

vec2 UV(vec3 pos)
{
	vec4 clip = projection * vec4(pos, 1.0);
    clip.xyz /= clip.w;

    return clip.xy * 0.5 + 0.5;
}

void BinarySearch(vec3 dir, vec3 pos)
{
    for(int i = 0; i < maxBinarySearchSteps; i++)
    {
        uv = UV(pos);
        if(any(greaterThan(uv, vec2(1.0))) || any(lessThan(uv, vec2(0.0))))
            break;
 
        float depth = ViewPosFromDepth(uv).z;
        float delta = pos.z - depth;

        if(abs(delta) < 0.01 && abs(depth) > depthBias)
            return;

        dir *= 0.5;
        pos += dir * sign(delta);
    }
 
    uv = vec2(-1.0);
}

vec3 SSR(vec3 dir, vec3 pos)
{
    dir *= rayStep;
 
    for(int i = 0; i < maxSteps; i++)
    {
        pos += dir;
 
        uv = UV(pos);
        if(any(greaterThan(uv, vec2(1.0))) || any(lessThan(uv, vec2(0.0))))
            return vec3(0.0);
 
        float depth = ViewPosFromDepth(uv).z;
 
        float delta = pos.z - depth;

        if((dir.z - delta) < 1.1)
        {
            if(delta <= 0.0)
            {
                if(maxBinarySearchSteps > 0)
                    BinarySearch(dir, pos);
                if(uv != vec2(-1.0) && abs(depth) > depthBias)
                    return texture(frame, uv).rgb;
                else
                    break;
            }
        }
    }

    return vec3(0.0);
}

void main()
{
    if(texture(depth, coord).r == 1.0)
        discard;

    vec4 combined = texture(gCombined, coord);

    if(combined.x == 0.0 && combined.y >= 0.9)
        discard;

    vec3 pos = ViewPosFromDepth(coord);

    vec3 normal = normalize(mat3(view) * texture(gNormal, coord).xyz);
         normal *= vec3(-1.0, 1.0, -1.0);

    vec3 viewDir = normalize(pos);
    vec3 reflected = normalize(reflect(viewDir, normal));
    
    vec3 ssr = SSR((reflected * max(0.1, -pos.z)), pos);

    fragColor = vec4(ssr, length(ssr) > 0.0 ? 1.0 : 0.0);
}
