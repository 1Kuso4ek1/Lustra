#version 450 core

uniform int albedoType;

uniform sampler2D albedoTexture;

uniform vec4 albedoValue;

in vec3 mPosition;
in vec3 mNormal;
in vec2 coord;

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gAlbedo;
layout(location = 2) out vec4 gNormal;

void main()
{
	gPosition = vec4(mPosition, 1.0f);

	if(albedoType == 0)
		gAlbedo = albedoValue;
	else
		gAlbedo = texture(albedoTexture, coord);

	gNormal = vec4(mNormal, 1.0f);
}
