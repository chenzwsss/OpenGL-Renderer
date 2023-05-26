#ifndef GLTF_MODEL_H
#define GLTF_MODEL_H

#include "tiny_gltf.h"

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gltfMesh.h"

#include "../graphic/gl_shader_program.h"

class gltf_model {
    public:
        // Contains the node's (optional) geometry and can be made up of an arbitrary number of primitives
        struct Mesh {
            std::vector<gltfMesh> primitives;
        };

        struct Image {
            unsigned int texture;
        };

        struct Texture {
            int32_t imageIndex;
        };

        struct Material {
            glm::vec4 baseColorFactor = glm::vec4(1.0f);
            uint32_t baseColorTextureIndex;
        };

        struct Node {
            Node* parent;
            std::vector<Node*> children;
            Mesh mesh;
            glm::mat4 matrix;
            ~Node() {
                for (auto& child : children) {
                    delete child;
                }
            }
        };

        gltf_model(const std::string file_path);

        void draw(gl_shader_program& shader);

        void drawNode(gltf_model::Node* node, gl_shader_program& shader);


        void load_gltf_file(const std::string file_path);
        void loadImages(tinygltf::Model& input);
        void loadTextures(tinygltf::Model& input);
        void loadMaterials(tinygltf::Model& input);
        void loadNode(const tinygltf::Node& input_node, const tinygltf::Model& input, gltf_model::Node* parent);

        /*
            Model data
        */
        std::vector<Image> images;
        std::vector<Texture> textures;
        std::vector<Material> materials;
        std::vector<Node*> m_nodes;
};

#endif