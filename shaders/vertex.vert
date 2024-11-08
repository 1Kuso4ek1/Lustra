#version 450 core

in vec2 position;
in vec2 texCoord;

out vec2 coord;

void main()
{
    coord = texCoord;

	gl_Position = vec4(position, 0.0f, 1.0f);
}
