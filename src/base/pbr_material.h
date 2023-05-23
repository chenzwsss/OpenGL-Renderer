#ifndef PBR_MATERIAL_H
#define PBR_MATERIAL_H

#include <glm/vec3.hpp>

#include <memory>
#include <array>
#include <string>

class pbr_material {
    public:
        pbr_material();

        enum parameter_type : int {
            ALBEDO = 0,
            AO,
            METALLIC,
            NORMAL,
            ROUGHNESS
        };

        void init(const std::string name,
            const std::string albedo_path,
            const std::string ao_path,
            const std::string metallic_path,
            const std::string normal_path,
            const std::string roughness_path,
            const std::string alpha_mask_path);

        void init(const std::string name,
            const glm::vec3& albedo,
            const glm::vec3& ao,
            const glm::vec3& metallic,
            const glm::vec3& normal,
            const glm::vec3& roughness,
            const float alpha = 1.0f);

        unsigned int get_parameter_texture(const parameter_type parameter) const;
        glm::vec3 get_parameter_color(const parameter_type parameter) const;

        auto get_alpha_value() const {
            return m_alpha;
        }

        auto get_alpha_mask() const {
            return m_alpha_mask_texture;
        }

        std::string m_name;
    private:
        std::array<unsigned int, 5> m_material_textures;
        std::array<glm::vec3, 5> m_material_colors;

        float m_alpha;
        unsigned int m_alpha_mask_texture;
};

using pbr_material_ptr = std::shared_ptr<pbr_material>;

#endif