#version 460 core

uniform sampler2D frame;

in vec2 coord;

out vec4 fragColor;

void main()
{
    vec2 texelSize = 1.0 / textureSize(frame, 0);

    float result = 0.0;
    
    for(int x = -2; x <= 2; x++)
    {
        for(int y = -2; y <= 2; y++)
        {
            result += texture(frame, coord + (vec2(x, y) * texelSize)).r;
        }
    }

    fragColor = vec4(vec3(result / 25.0), 1.0);
}
