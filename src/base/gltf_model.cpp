#include "gltf_model.h"

#include <glad/glad.h>

#include <iostream>

#include "../utility/resource_manager.h"
#include "../base/Vertex.h"

gltf_model::gltf_model(const std::string file_path) {
	load_gltf_file(file_path);
}

void gltf_model::load_gltf_file(const std::string file_path) {
	std::string new_path = resource_manager::get_instance().get_assets_path() + file_path;

    tinygltf::Model gltf_input;
    tinygltf::TinyGLTF gltf_content;
    std::string error, warning;

    bool file_loaded = gltf_content.LoadASCIIFromFile(&gltf_input, &error, &warning, new_path);

	if (file_loaded) {
		loadImages(gltf_input);
		loadMaterials(gltf_input);
		loadTextures(gltf_input);
		const tinygltf::Scene& scene = gltf_input.scenes[0];
		for (size_t i = 0; i < scene.nodes.size(); i++) {
			const tinygltf::Node node = gltf_input.nodes[scene.nodes[i]];
			loadNode(node, gltf_input, nullptr);
		}
	} else {
		std::cerr << "Could not open the glTF file: " << file_path << "error: " << error << std::endl;
	}
}

void gltf_model::draw(gl_shader_program& shader) {
    for (auto& node : m_nodes) {
        drawNode(node, shader);
    }
}

void gltf_model::drawNode(gltf_model::Node* node, gl_shader_program& shader) {
    if (node->mesh.primitives.size() > 0) {
 
		// Set model matrix
        shader.set_uniform("modelMatrix", node->matrix);

        for (auto& primitive : node->mesh.primitives) {
            if (primitive.m_indexCount > 0) {

				uint32_t indexx = materials[primitive.m_materialIndex].baseColorTextureIndex;
                gltf_model::Texture texture = textures[materials[primitive.m_materialIndex].baseColorTextureIndex];
                glActiveTexture(GL_TEXTURE0);
	            glBindTexture(GL_TEXTURE_2D, images[texture.imageIndex].texture);

                primitive.draw();
                
            }
        }
    }

	for (auto& child : node->children) {
		drawNode(child, shader);
	}
}

void gltf_model::loadImages(tinygltf::Model& input) {
	// Images can be stored inside the glTF (which is the case for the sample model), so instead of directly
	// loading them from disk, we fetch them from the glTF loader and upload the buffers
	images.resize(input.images.size());
	for (size_t i = 0; i < input.images.size(); i++) {
		tinygltf::Image& glTFImage = input.images[i];
		// Get the image data from the glTF loader
		unsigned char* buffer = nullptr;
		uint64_t bufferSize = 0;
		bool deleteBuffer = false;
		// We convert RGB-only images to RGBA, as most devices don't support RGB-formats in Vulkan
		if (glTFImage.component == 3) {
			bufferSize = glTFImage.width * glTFImage.height * 4;
			buffer = new unsigned char[bufferSize];
			unsigned char* rgba = buffer;
			unsigned char* rgb = &glTFImage.image[0];
			for (size_t i = 0; i < glTFImage.width * glTFImage.height; ++i) {
				memcpy(rgba, rgb, sizeof(unsigned char) * 3);
				rgba += 4;
				rgb += 3;
			}
			deleteBuffer = true;
		}
		else {
			buffer = &glTFImage.image[0];
			bufferSize = glTFImage.image.size();
		}
		// Load texture from image buffer
		images[i].texture = resource_manager::get_instance().texture_from_buffer(
			buffer,
			glTFImage.name,
			glTFImage.width,
			glTFImage.height,
			glTFImage.component,
			true
		);
		if (deleteBuffer) {
			delete[] buffer;
		}
	}
}

void gltf_model::loadTextures(tinygltf::Model& input) {
	textures.resize(input.textures.size());
	for (size_t i = 0; i < input.textures.size(); ++i) {
		textures[i].imageIndex = input.textures[i].source;
	}
}

void gltf_model::loadMaterials(tinygltf::Model& input) {
	materials.resize(input.materials.size());
	for (size_t i = 0; i < input.materials.size(); ++i) {
		tinygltf::Material glTFMaterial = input.materials[i];
		// Get base color texture index
		if (glTFMaterial.values.find("baseColorTexture") != glTFMaterial.values.end()) {
			materials[i].baseColorTextureIndex = glTFMaterial.values["baseColorTexture"].TextureIndex();
		}
	}
}

void gltf_model::loadNode(const tinygltf::Node& input_node, const tinygltf::Model& input, gltf_model::Node* parent) {
	gltf_model::Node* node = new gltf_model::Node();
	node->matrix = glm::mat4(1.0f);
	node->parent = parent;

	// Get the local node matrix
	// It's either made up from translation, rotation, scale or a 4x4 matrix
	if (input_node.translation.size() == 3) {
		node->matrix = glm::translate(node->matrix, glm::vec3(glm::make_vec3(input_node.translation.data())));
	}
	if (input_node.rotation.size() == 4) {
		glm::quat q = glm::make_quat(input_node.rotation.data());
		node->matrix *= glm::mat4(q);
	}
	if (input_node.scale.size() == 3) {
		node->matrix = glm::scale(node->matrix, glm::vec3(glm::make_vec3(input_node.scale.data())));
	}
	if (input_node.matrix.size() == 16) {
		node->matrix = glm::make_mat4x4(input_node.matrix.data());
	};

	// Load node's children
	if (input_node.children.size() > 0) {
		for (size_t i = 0; i < input_node.children.size(); i++) {
			loadNode(input.nodes[input_node.children[i]], input, node);
		}
	}

	// If the node contains mesh data, we load vertices and indices from the buffers
	// In glTF this is done via accessors and buffer views
	if (input_node.mesh > -1) {
		const tinygltf::Mesh mesh = input.meshes[input_node.mesh];

		// Iterate through all primitives of this node's mesh
		for (size_t i = 0; i < mesh.primitives.size(); ++i) {
			const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];

            std::vector<Vertex> vertices;
			// Vertices
			{
				const float* positionBuffer = nullptr;
				const float* normalsBuffer = nullptr;
				const float* texCoordsBuffer = nullptr;
				size_t vertexCount = 0;

				// Get buffer data for vertex positions
				if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end()) {
					const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("POSITION")->second];
					const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
					positionBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					vertexCount = accessor.count;
				}
				// Get buffer data for vertex normals
				if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end()) {
					const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
					const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
					normalsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}
				// Get buffer data for vertex texture coordinates
				// glTF supports multiple sets, we only load the first one
				if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end()) {
					const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
					const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
					texCoordsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}

				// Append data to model's vertex buffer
				for (size_t v = 0; v < vertexCount; v++) {
					Vertex vert{};
					vert.Position = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
					vert.Normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
					vert.TexCoords = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
					vertices.push_back(vert);
				}
			}

            std::vector<GLuint> indices;
            // Indices
            {
                const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.indices];
                const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];

                // glTF supports different component types of indices
                switch (accessor.componentType) {
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                        const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                        for (size_t index = 0; index < accessor.count; index++) {
                            indices.push_back(buf[index]);
                        }
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                        const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                        for (size_t index = 0; index < accessor.count; index++) {
                            indices.push_back(buf[index]);
                        }
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                        const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                        for (size_t index = 0; index < accessor.count; index++) {
                            indices.push_back(buf[index]);
                        }
                        break;
                    }
                    default:
                        std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
                        return;
                }
            }

            gltfMesh primitive(vertices, indices, glTFPrimitive.material);
            node->mesh.primitives.push_back(primitive);
		}
	}

	if (parent) {
		parent->children.push_back(node);
	}
	else {
		m_nodes.push_back(node);
	}
}
