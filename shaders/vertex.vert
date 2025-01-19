#version 460 core

layout(std140) uniform matrices
{
    mat4 model, view, projection;
};

in vec3 position;
in vec3 normal;
in vec2 texCoord;

out vec3 mPosition;
out vec3 mNormal;
out mat3 TBN;
out vec2 coord;

void main()
{
    mPosition = (model * vec4(position, 1.0f)).xyz;
    mNormal = normalize(mat3(model) * normal).xyz;
    coord = texCoord;

    vec3 tangent = cross(mNormal, vec3(0.5, 0.5, 0.5));
    vec3 T = normalize(mat3(view * model) * tangent);
    vec3 N = mNormal;
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);
    
	gl_Position = projection * view * model * vec4(position, 1.0f);
}
