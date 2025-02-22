#version 460 core

uniform sampler2D frame;

uniform bool horizontal;

in vec2 coord;

out vec4 fragColor;

const float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
    vec2 texelSize = 1.0 / textureSize(frame, 0);
    vec3 result = texture(frame, coord).rgb * weights[0];

    const vec2 horizontalOffset = vec2(texelSize.x, 0.0);
    const vec2 verticalOffset = vec2(0.0, texelSize.y);

    if(horizontal)
    {
        for(int i = 1; i < 5; i++)
        {
            result += clamp(texture(frame, coord + (horizontalOffset * i)).rgb, vec3(0.0), vec3(50.0)) * weights[i];
            result += clamp(texture(frame, coord - (horizontalOffset * i)).rgb, vec3(0.0), vec3(50.0)) * weights[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; i++)
        {
            result += clamp(texture(frame, coord + (verticalOffset * i)).rgb, vec3(0.0), vec3(50.0)) * weights[i];
            result += clamp(texture(frame, coord - (verticalOffset * i)).rgb, vec3(0.0), vec3(50.0)) * weights[i];
        }
    }

    fragColor = vec4(result, 1.0);
}
