#ifndef GLTF_MESH_H
#define GLTF_MESH_H

#include "Vertex.h"
#include "../graphic/GLVertexArray.h"
#include "../graphic/GLShaderProgram.h"

#include <vector>

class glTFMesh {
    public:
        glTFMesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, int32_t materialIndex);

        void setupMesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);

        void draw();

        int32_t m_materialIndex;
        uint32_t m_indexCount;
        GLVertexArray m_VAO;
};

#endif