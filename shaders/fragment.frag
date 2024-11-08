#version 450 core

uniform sampler2D albedo;

in vec2 coord;

out vec4 fragColor;

void main()
{
	fragColor = texture(albedo, coord);
}
