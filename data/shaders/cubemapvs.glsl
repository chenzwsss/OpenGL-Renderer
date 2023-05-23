#version 330 core

layout (location = 0) in vec3 aPosition;

uniform mat4 projection;
uniform mat4 view;

out vec3 WorldPos;

void main() {
    WorldPos = aPosition;
    gl_Position =  projection * view * vec4(WorldPos, 1.0);
}