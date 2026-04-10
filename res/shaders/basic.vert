#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform vec2 uScale;
uniform vec2 uCenter;

void main() {
    vec2 scaledPos = (aPos - uCenter) * uScale + uCenter;
    gl_Position = vec4(scaledPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}