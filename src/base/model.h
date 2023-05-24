#ifndef MODEL_H
#define MODEL_H

#include <glm/mat4x4.hpp>

#include <memory>
#include <string>

#include "base_mesh.h"
#include "../graphic/gl_shader_program.h"

struct aiScene;
struct aiNode;
struct aiMesh;

class model {
    public:
        model() = default;
        model(const std::string path, const std::string name, const bool flip_winding_order = false, const bool load_material = true);
        model(const std::string name, const std::vector<vertex>& vertices, const std::vector<GLuint>& indices, const pbr_material_ptr& material);
        model(const std::string name, const base_mesh& mesh);
        virtual ~model() = default;

        void attach_mesh(const base_mesh mesh);

        void scale(const glm::vec3& scale);
        void rotate(const float degrees, const glm::vec3& axis);
        void translate(const glm::vec3& pos);

        glm::mat4 get_model_matrix() const;

        void draw(gl_shader_program& shader);

        void destroy();

        auto get_meshes() const { return m_meshes; }
    protected:
        std::vector<base_mesh> m_meshes;

    private:
        bool load_model(const std::string path, const bool flip_winding_order, const bool load_material);
        void process_node(aiNode* node, const aiScene* scene, const bool load_material);
        base_mesh process_mesh(aiMesh* mesh, const aiScene* scene, const bool load_material);

        glm::vec3 m_scale, m_position, m_axis;
        float m_degrees;

        const std::string m_name;
        std::string m_path;

        std::size_t m_num_mats;
};

#endif