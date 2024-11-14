#version 450 core

layout(std140) uniform matrices
{
    mat4 model, view, projection;
};

in vec3 position;
in vec3 normal;
in vec2 texCoord;

out vec3 mNormal;
out vec2 coord;

void main()
{
    mNormal = mat3(model) * normal;
    coord = texCoord;
    
	gl_Position = projection * view * model * vec4(position, 1.0f);
}
