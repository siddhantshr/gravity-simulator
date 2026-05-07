#version 330 core

in vec2 localPos;

out vec4 FragColor;

void main() {

    float d = length(localPos);

    if (d > 1.0)
        discard;

    vec2 lightDir = normalize(vec2(-0.4, 0.8));

    vec3 normal = normalize(vec3(
        localPos,
        sqrt(1.0 - d*d)
    ));

    float lighting = dot(
        normal,
        normalize(vec3(lightDir, 1.0))
    );

    lighting = max(lighting, 0.0);

    // icy blue base
    vec3 base = vec3(0.75, 0.9, 1.0);

    vec3 color = base * (0.15 + lighting);

    // soft cold glow
    float glow = exp(-3.5 * d);

    color += vec3(0.85, 0.97, 1.0) * glow * 0.35;

    // bright icy rim
    float rim = pow(1.0 - d, 4.0);

    color += vec3(0.9, 1.0, 1.0) * rim * 0.5;

    // soft edges
    float alpha = smoothstep(1.0, 0.96, d);

    FragColor = vec4(color, alpha);
}