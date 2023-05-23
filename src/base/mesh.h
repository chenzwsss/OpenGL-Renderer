#ifndef MESH_H
#define MESH_H

#include "vertex.h"
#include "../graphic/gl_vertex_array.h"
#include "pbr_material.h"

#include <vector>

struct mesh {
    mesh(const std::vector<vertex>& vertices, const std::vector<GLuint>& indices);
    mesh(const std::vector<vertex>& vertices, const std::vector<GLuint>& indices, const pbr_material_ptr& material);

    void clear();

    auto get_triangle_count() const { return indexCount / 3; }

    const std::size_t indexCount;
    gl_vertex_array vao;
    pbr_material_ptr material;

    private:
        void setup_mesh(const std::vector<vertex>& vertices, const std::vector<GLuint>& indices);
};

#endif