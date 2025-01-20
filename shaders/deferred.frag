#version 460 core

uniform int albedoType;
uniform int normalType;
uniform int metallicType;
uniform int roughnessType;
uniform int aoType;
uniform int emissionType;

uniform sampler2D albedoTexture;
uniform sampler2D normalTexture;
uniform sampler2D metallicTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D aoTexture;
uniform sampler2D emissionTexture;

uniform vec4 albedoValue;
uniform float metallicValue;
uniform float roughnessValue;
uniform vec3 emissionValue;
uniform float emissionStrength;

in vec3 mPosition;
in vec3 mNormal;
in mat3 TBN;
in vec2 coord;

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gAlbedo;
layout(location = 2) out vec4 gNormal;
layout(location = 3) out vec4 gCombined;
layout(location = 4) out vec4 gEmission;

void main()
{
	float metallic = metallicValue;
	float roughness = roughnessValue;
	float ao = 1.0;

	gPosition = vec4(mPosition, 1.0);

	if(albedoType == 0)
		gAlbedo = albedoValue;
	else
		gAlbedo = texture(albedoTexture, coord);

	if(normalType == 0)
		gNormal = vec4(mNormal, 1.0);
	else
		gNormal = vec4(normalize(TBN * normalize(texture(normalTexture, coord).xyz * 2.0 - 1.0)), 1.0);

	if(metallicType == 1)
		metallic = texture(metallicTexture, coord).b;
	if(roughnessType == 1)
		roughness = texture(roughnessTexture, coord).g;
	if(aoType == 1)
		ao = texture(aoTexture, coord).r;

	if(emissionType == 0)
		gEmission = vec4(emissionValue * emissionStrength, 1.0);
	else
		gEmission = vec4(texture(emissionTexture, coord).rgb * emissionStrength, 1.0);

	gCombined = vec4(metallic, roughness, ao, 1.0);
}
