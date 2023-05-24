#ifndef BASE_MESH_H
#define BASE_MESH_H

#include "vertex.h"
#include "../graphic/gl_vertex_array.h"
#include "../graphic/gl_shader_program.h"
#include "pbr_material.h"

#include <vector>

struct base_mesh {
    base_mesh(const std::vector<vertex>& vertices, const std::vector<GLuint>& indices);
	base_mesh(const std::vector<vertex>& vertices, const std::vector<GLuint>& indices, const pbr_material_ptr& material);

    void draw(gl_shader_program& shader);

    auto get_triangle_count() const { return index_count / 3; }

    const std::size_t index_count;
    gl_vertex_array vao;
    pbr_material_ptr material;

    private:
        void setup_mesh(const std::vector<vertex>& vertices, const std::vector<GLuint>& indices);
};

#endif