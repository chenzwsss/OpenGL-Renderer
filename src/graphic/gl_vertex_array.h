#ifndef GL_VERTEX_ARRAY_H
#define GL_VERTEX_ARRAY_H

#include <glad/glad.h>

class gl_vertex_array {
    public:
        enum BufferType : int {
            ARRAY = GL_ARRAY_BUFFER,
            ELEMENT = GL_ELEMENT_ARRAY_BUFFER
        };

        enum DrawMode : int {
            STATIC = GL_STATIC_DRAW,
            DYNAMIC = GL_DYNAMIC_DRAW,
            STREAM = GL_STREAM_DRAW
        };

        void init();
        void attachBuffer(const BufferType type, const size_t size, const DrawMode mode, const void* data);
        void bind() const;
        void enableAttribute(const GLuint index, const int size, const GLuint offset, const void* data);
        void destroy();

    private:
        GLuint m_vao { 0 };
};

#endif