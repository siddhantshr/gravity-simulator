#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in float aAlpha;

uniform mat4 vp;

uniform vec2 circleCenter;
uniform float radius;

out vec2 localPos;

void main() {

    localPos = (aPos - circleCenter) / radius;

    gl_Position = vp * vec4(aPos, 0.0, 1.0);
}