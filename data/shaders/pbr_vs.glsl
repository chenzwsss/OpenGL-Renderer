#version 420 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

layout (std140, binding = 0) uniform Matrices {
    mat4 projection;
    mat4 view;
};

uniform mat4 modelMatrix;

out VertexData {
    out vec3 vWorldPos;
    out vec3 vNormal;
    out vec2 vTexCoords;
} vertexData;

void main() {
    vertexData.vTexCoords = aTexCoords;

    vertexData.vWorldPos = vec3(modelMatrix * vec4(aPosition, 1.0));
    vertexData.vNormal = mat3(modelMatrix) * aNormal;

    gl_Position = projection * view * vec4(vertexData.vWorldPos, 1.0);
}
