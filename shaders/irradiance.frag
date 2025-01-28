#version 460 core

const float PI = 3.14159265359;

uniform samplerCubeArray skybox;

in vec3 vertex;

out vec4 fragColor;

void main()
{
    vec3 normal = normalize(vertex);
    vec3 irradiance = vec3(0.0);

    vec3 right = normalize(cross(vec3(0.0, 1.0, 0.0), normal));
    vec3 up = normalize(cross(normal, right));

    float sampleDelta = (PI * 2.0) / 200;//0.025;
    float nrSamples = 0.0;
    
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

            irradiance += texture(skybox, vec4(sampleVec, 0.0)).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }

    irradiance = PI * irradiance * (1.0 / float(nrSamples));

    fragColor = vec4(irradiance, 1.0);
}
