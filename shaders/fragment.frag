#version 450 core

uniform sampler2D albedo;

in vec3 mPosition;
in vec3 mNormal;
in vec2 coord;

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gAlbedo;
layout(location = 2) out vec4 gNormal;

void main()
{
	gPosition = vec4(mPosition, 1.0f);
	gAlbedo = texture(albedo, coord);
	gNormal = vec4(mNormal, 1.0f);
}
