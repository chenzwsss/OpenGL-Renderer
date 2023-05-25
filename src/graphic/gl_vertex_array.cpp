#include "gl_vertex_array.h"

void gl_vertex_array::init() {
    glGenVertexArrays(1, &m_vao);
}

void gl_vertex_array::attach_buffer(const buffer_type type, const size_t size, const draw_mode mode, const void* data) {
    GLuint buffer;
    glGenBuffers(1, &buffer);

    glBindBuffer(type, buffer);
    glBufferData(type, size, data, mode);
}

void gl_vertex_array::bind() const {
    glBindVertexArray(m_vao);
}

void gl_vertex_array::unbind() const {
    glBindVertexArray(0);
}

void gl_vertex_array::enable_attribute(const GLuint index, const int size, const GLuint offset, const void* data) {
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, offset, data);
}

void gl_vertex_array::destroy() {
    glDeleteVertexArrays(1, &m_vao);
}