#version 450 core

layout(std140) uniform matrices
{
    mat4 model, view, projection;
};

in vec3 position;

out vec3 vertex;
out vec3 sun;

uniform float time = 100.0;

void main()
{
    vertex = position;
    sun = vec3(0.0, sin(time * 0.01), cos(time * 0.01));

    gl_Position = ((projection * mat4(mat3(view))) * vec4(position, 1.0)).xyww;
}
