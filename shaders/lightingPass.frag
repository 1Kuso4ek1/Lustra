#version 450 core

uniform sampler2D gPosition;
uniform sampler2D gAlbedo;
uniform sampler2D gNormal;

in vec2 coord;

out vec4 fragColor;

vec3 lightPosition = vec3(3.0, 3.0, 3.0);
vec3 lightColor = vec3(1.0, 1.0, 1.0);

float lightIntensity = 1.0;

float ambientStrength = 0.1;

void main()
{
    vec3 worldPosition = texture(gPosition, coord).xyz;
    vec3 albedo = texture(gAlbedo, coord).rgb;
    vec3 normal = normalize(texture(gNormal, coord).xyz);

    vec3 lightDir = normalize(lightPosition - worldPosition);

    vec3 diffuse = max(dot(normal, lightDir), 0.0) * lightColor * lightIntensity;

    vec3 ambient = ambientStrength * lightColor;

    vec3 finalColor = (ambient + diffuse) * albedo;

    fragColor = vec4(finalColor, 1.0);
}
