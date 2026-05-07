#version 330 core

in vec2 localPos;

out vec4 FragColor;

void main() {

    float d = length(localPos);

    if (d > 1.0)
        discard;

    vec2 lightDir = normalize(vec2(-0.5, 0.7));

    vec3 normal = normalize(vec3(
        localPos,
        sqrt(1.0 - d*d)
    ));

    float lighting = dot(
        normal,
        normalize(vec3(lightDir, 1.0))
    );

    lighting = max(lighting, 0.0);

    vec3 base = vec3(1.0, 0.3, 0.1);

    vec3 color = base * (0.2 + lighting);

    float glow = exp(-3.0 * d);

    color += base * glow * 0.3;

    FragColor = vec4(color, 1.0);
}