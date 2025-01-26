#version 460 core

uniform sampler2D frame;

uniform float threshold;
uniform float softThreshold = 0.7;

in vec2 coord;

out vec4 fragColor;

vec4 DownsampleBox13Tap(vec2 texelSize)
{
    vec4 A = texture(frame, coord + texelSize * vec2(-1.0, -1.0));
    vec4 B = texture(frame, coord + texelSize * vec2(0.0, -1.0));
    vec4 C = texture(frame, coord + texelSize * vec2(1.0, -1.0));
    vec4 D = texture(frame, coord + texelSize * vec2(-0.5, -0.5));
    vec4 E = texture(frame, coord + texelSize * vec2(0.5, -0.5));
    vec4 F = texture(frame, coord + texelSize * vec2(-1.0, 0.0));
    vec4 G = texture(frame, coord);
    vec4 H = texture(frame, coord + texelSize * vec2(1.0, 0.0));
    vec4 I = texture(frame, coord + texelSize * vec2(-0.5, 0.5));
    vec4 J = texture(frame, coord + texelSize * vec2(0.5, 0.5));
    vec4 K = texture(frame, coord + texelSize * vec2(-1.0, 1.0));
    vec4 L = texture(frame, coord + texelSize * vec2(0.0, 1.0));
    vec4 M = texture(frame, coord + texelSize * vec2(1.0, 1.0));

    vec2 div = (1.0 / 4.0) * vec2(0.5, 0.125);

    vec4 o = (D + E + I + J) * div.x;
         o += (A + B + G + F) * div.y;
         o += (B + C + H + G) * div.y;
         o += (F + G + L + K) * div.y;
         o += (G + H + M + L) * div.y;

    return o;
}

void main()
{
    vec3 color = texture(frame, coord).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));

    float knee = threshold * softThreshold;
    float soft = brightness - threshold + knee;
    soft = clamp(soft, 0.0, 2.0 * knee);
    soft = soft * soft / (4.0 * knee + 1e-4);

    float contribution = max(soft, brightness - threshold);
    contribution /= max(brightness, 1e-4);

    fragColor = vec4(color * contribution, 1.0);
}
