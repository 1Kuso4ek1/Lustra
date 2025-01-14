#version 460 core

uniform sampler2D frame;

in vec2 coord;

out vec4 fragColor;

uniform float exposure;

mat3 acesIn = mat3(0.59719, 0.07600, 0.02840,
                   0.35458, 0.90834, 0.13383,
                   0.04823, 0.01566, 0.83777);

mat3 acesOut = mat3(1.60475, -0.10208, -0.00327,
                    -0.53108,  1.10813, -0.07276,
                    -0.07367, -0.00605, 1.07602);

vec3 ACES(vec3 color)
{
    vec3 v = acesIn * color * exposure;
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return acesOut * (a / b);
}

void main()
{
    vec4 color = texture(frame, coord);
    color.rgb = ACES(color.rgb);
    color.rgb = pow(color.rgb, vec3(1.0 / 2.2));

    fragColor = color;
}
