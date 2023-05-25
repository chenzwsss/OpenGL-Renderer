#include "base_mesh.h"

#include <iostream>

base_mesh::base_mesh(const std::vector<vertex>& vertices, const std::vector<GLuint>& indices)
	: index_count(indices.size())
{
	setup_mesh(vertices, indices);
}

base_mesh::base_mesh(const std::vector<vertex>& vertices, const std::vector<GLuint>& indices, const pbr_material_ptr& material)
	: index_count(indices.size()), material(material)
{
	setup_mesh(vertices, indices);
}

void base_mesh::draw(gl_shader_program& shader) {
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, this->material->get_parameter_texture(pbr_material::ALBEDO));
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, this->material->get_parameter_texture(pbr_material::NORMAL));
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, this->material->get_parameter_texture(pbr_material::METALLIC));
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, this->material->get_parameter_texture(pbr_material::ROUGHNESS));

	vao.bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(index_count), GL_UNSIGNED_INT, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);
	vao.unbind();
}

void base_mesh::setup_mesh(const std::vector<vertex>& vertices, const std::vector<GLuint>& indices) {
	
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
	// Normal
	vao.enable_attribute(1, 3, vertex_size, reinterpret_cast<void*>(offsetof(vertex, Normal)));
	// Texture Coord 0
	vao.enable_attribute(2, 2, vertex_size, reinterpret_cast<void*>(offsetof(vertex, TexCoords)));
	// Tangent
	vao.enable_attribute(3, 3, vertex_size, reinterpret_cast<void*>(offsetof(vertex, Tangent)));
	// Bitangent
	vao.enable_attribute(4, 3, vertex_size, reinterpret_cast<void*>(offsetof(vertex, Bitangent)));
}