#include "mesh.h"

mesh::mesh(const std::vector<vertex>& vertices, const std::vector<GLuint>& indices) : index_count(indices.size()) {

	setup_mesh(vertices, indices);
}

mesh::mesh(const std::vector<vertex>& vertices, const std::vector<GLuint>& indices, const pbr_material_ptr& material) :
	index_count(indices.size()),
	material(material) {

	setup_mesh(vertices, indices);
}

void mesh::setup_mesh(const std::vector<vertex>& vertices, const std::vector<GLuint>& indices) {
	
	vao.init();
	vao.bind();
	// Attach VBO
	vao.attach_buffer(gl_vertex_array::buffer_type::ARRAY, vertices.size() * sizeof(vertex), gl_vertex_array::draw_mode::STATIC, &vertices[0]);
	// Attach EBO
	vao.attach_buffer(gl_vertex_array::buffer_type::ELEMENT, indices.size() * sizeof(GLuint), gl_vertex_array::draw_mode::STATIC, &indices[0]);

	// Vertex Attributes

	const static auto vertex_size = sizeof(vertex);
	// Position
	vao.enable_attribute(0, 3, vertex_size, nullptr);
	// Texture Coords
	vao.enable_attribute(1, 2, vertex_size, reinterpret_cast<void*>(offsetof(vertex, TexCoords)));
	// Normal
	vao.enable_attribute(2, 3, vertex_size, reinterpret_cast<void*>(offsetof(vertex, Normal)));
	// Tangent
	vao.enable_attribute(3, 3, vertex_size, reinterpret_cast<void*>(offsetof(vertex, Tangent)));
}