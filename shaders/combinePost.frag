#version 460 core

uniform sampler2D frame;

in vec2 coord;

out vec4 fragColor;

void main()
{
    fragColor = vec4(texture(frame, coord).rgb, 1.0);
}
