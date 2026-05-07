#version 330 core

layout (location = 0) out vec4 color;

in float alpha; // alpha value passed from vertex shader

void main() {
    vec3 core = vec3(1.0,0.8,0.3);
    vec3 edge = vec3(1.0,0.3,0.0);

    vec3 final = mix(edge, core, alpha);

    color = vec4(final, alpha * 0.8);
}