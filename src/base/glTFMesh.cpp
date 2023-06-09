#include "glTFMesh.h"

glTFMesh::glTFMesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, int32_t materialIndex)
: m_materialIndex(materialIndex), m_indexCount(static_cast<uint32_t>(indices.size())) {
    setupMesh(vertices, indices);
}

void glTFMesh::setupMesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices) {
    m_VAO.init();
    m_VAO.bind();
    // Attach VBO
    m_VAO.attachBuffer(GLVertexArray::buffer_type::ARRAY, vertices.size() * sizeof(Vertex), GLVertexArray::draw_mode::STATIC, &vertices[0]);
    // Attach EBO
    m_VAO.attachBuffer(GLVertexArray::buffer_type::ELEMENT, indices.size() * sizeof(GLuint), GLVertexArray::draw_mode::STATIC, &indices[0]);

    // Vertex Attributes
    const static auto vertex_size = sizeof(Vertex);
    // Position
    m_VAO.enableAttribute(0, 3, vertex_size, nullptr);
    // Normal
    m_VAO.enableAttribute(1, 3, vertex_size, reinterpret_cast<void*>(offsetof(Vertex, Normal)));
    // Texture Coord 0
    m_VAO.enableAttribute(2, 2, vertex_size, reinterpret_cast<void*>(offsetof(Vertex, TexCoords)));
}

void glTFMesh::draw() {
    m_VAO.bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexCount), GL_UNSIGNED_INT, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    m_VAO.unbind();
}