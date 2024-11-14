#version 450 core

uniform sampler2D albedo;

in vec3 mNormal;
in vec2 coord;

out vec4 fragColor;

void main()
{
	fragColor = texture(albedo, coord) + (vec4(mNormal, 0.0) / 5.0);
}
