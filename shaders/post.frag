#version 450 core

in vec2 coord;

out vec4 color;

uniform sampler2D frame;

void main()
{
    color = texture(frame, coord);
    
    color.rgb = pow(color.rgb, vec3(1.0 / 2.2));
}
