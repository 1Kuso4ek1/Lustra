#version 460 core

layout(std140) uniform matrices
{
    mat4 model, view, projection;
};

in vec3 position;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f);
}
