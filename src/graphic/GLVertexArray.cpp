#include "GLVertexArray.h"

void GLVertexArray::init() {
    glGenVertexArrays(1, &m_vao);
}

void GLVertexArray::attachBuffer(const buffer_type type, const size_t size, const draw_mode mode, const void* data) {
    GLuint buffer;
    glGenBuffers(1, &buffer);

    glBindBuffer(type, buffer);
    glBufferData(type, size, data, mode);
}

void GLVertexArray::bind() const {
    glBindVertexArray(m_vao);
}

void GLVertexArray::unbind() const {
    glBindVertexArray(0);
}

void GLVertexArray::enableAttribute(const GLuint index, const int size, const GLuint offset, const void* data) {
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, offset, data);
}

void GLVertexArray::destroy() {
    glDeleteVertexArrays(1, &m_vao);
}