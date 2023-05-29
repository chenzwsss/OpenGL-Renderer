#ifndef GLTF_MODEL_H
#define GLTF_MODEL_H

#include "tiny_gltf.h"

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glTFMesh.h"

#include "../graphic/GLShaderProgram.h"

class glTFModel {
    public:
        // Contains the node's (optional) geometry and can be made up of an arbitrary number of primitives
        struct Mesh {
            std::vector<glTFMesh> primitives;
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

        glTFModel(const std::string file_path);

        void draw(GLShaderProgram& shader);

        void drawNode(glTFModel::Node* node, GLShaderProgram& shader);


        void load_gltf_file(const std::string file_path);
        void loadImages(tinygltf::Model& input);
        void loadTextures(tinygltf::Model& input);
        void loadMaterials(tinygltf::Model& input);
        void loadNode(const tinygltf::Node& input_node, const tinygltf::Model& input, glTFModel::Node* parent);

        /*
            Model data
        */
        std::vector<Image> images;
        std::vector<Texture> textures;
        std::vector<Material> materials;
        std::vector<Node*> m_nodes;
};

#endif