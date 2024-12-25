#version 450 core

layout(std140) uniform matrices
{
    mat4 model, view, projection;
};

in vec3 position;
in vec3 normal;
in vec2 texCoord;

out vec3 mPosition;
out vec3 mNormal;
out vec2 coord;

void main()
{
    mPosition = (model * vec4(position, 1.0f)).xyz;
    mNormal = (model * vec4(normal, 1.0f)).xyz;
    coord = texCoord;
    
	gl_Position = projection * view * model * vec4(position, 1.0f);
}
