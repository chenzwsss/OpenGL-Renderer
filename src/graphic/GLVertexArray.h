#ifndef GL_VERTEX_ARRAY_H
#define GL_VERTEX_ARRAY_H

#include <glad/glad.h>

class GLVertexArray {
    public:
        enum buffer_type : int {
            ARRAY = GL_ARRAY_BUFFER,
            ELEMENT = GL_ELEMENT_ARRAY_BUFFER
        };

        enum draw_mode : int {
            STATIC = GL_STATIC_DRAW,
            DYNAMIC = GL_DYNAMIC_DRAW,
            STREAM = GL_STREAM_DRAW
        };

        void init();
        void attach_buffer(const buffer_type type, const size_t size, const draw_mode mode, const void* data);
        void bind() const;
        void unbind() const;
        void enable_attribute(const GLuint index, const int size, const GLuint offset, const void* data);
        void destroy();

    private:
        GLuint m_vao { 0 };
};

#endif