#ifndef GLTF_MESH_H
#define GLTF_MESH_H

#include "Vertex.h"
#include "../graphic/gl_vertex_array.h"
#include "../graphic/gl_shader_program.h"

#include <vector>

class glTFMesh {
    public:
        glTFMesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, int32_t materialIndex);

        void setupMesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);

        void draw();

        int32_t m_materialIndex;
        uint32_t m_indexCount;
        gl_vertex_array m_VAO;
};

#endif