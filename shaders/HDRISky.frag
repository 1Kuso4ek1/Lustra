#version 460 core

uniform samplerCubeArray skybox;

in vec3 vertex;

out vec4 fragColor;

void main()
{
    vec3 coords = normalize(vertex);

    fragColor = vec4(texture(skybox, vec4(coords, 0.0)).rgb, 1.0);
}

