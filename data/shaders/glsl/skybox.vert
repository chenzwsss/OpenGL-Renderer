#version 420 core

layout (location = 0) in vec3 aPosition;

layout (std140, binding = 0) uniform Matrices {
    mat4 projection;
    mat4 view;
};

out vec3 WorldPos;

void main() {
    WorldPos = aPosition;

    vec4 clipPos = projection * mat4(mat3(view)) * vec4(aPosition, 1.0);

    gl_Position = clipPos.xyww;
}
