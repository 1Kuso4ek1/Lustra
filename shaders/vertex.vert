#version 450 core

in vec2 position;
in vec3 color;

out vec3 vertexColor;

void main()
{
    vertexColor = color;
    
	gl_Position = vec4(position, 0.0f, 1.0f);
}
