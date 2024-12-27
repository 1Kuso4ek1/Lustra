#version 450 core

in vec3 position;
in vec2 texCoord;

out vec2 coord;

void main()
{
    coord = texCoord;

	gl_Position = vec4(position, 1.0f);
}
