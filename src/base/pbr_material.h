#ifndef PBR_MATERIAL_H
#define PBR_MATERIAL_H

#include <glm/vec3.hpp>

#include <memory>
#include <array>
#include <string>

class pbr_material {
    public:
        pbr_material();

        enum ParameterType : int {
            ALBEDO = 0,
            AO,
            METALLIC,
            NORMAL,
            ROUGHNESS
        };

        void init(const std::string name,
            const std::string albedoPath,
            const std::string aoPath,
            const std::string metallicPath,
            const std::string normalPath,
            const std::string roughnessPath,
            const std::string alphaMaskPath);

        void init(const std::string name,
            const glm::vec3& albedo,
            const glm::vec3& ao,
            const glm::vec3& metallic,
            const glm::vec3& normal,
            const glm::vec3& roughness,
            const float alpha = 1.0f);

        unsigned int get_parameter_texture(const ParameterType parameter) const;
        glm::vec3 get_parameter_color(const ParameterType parameter) const;

        auto get_alpha_value() const {
            return m_alpha;
        }

        auto get_alpha_mask() const {
            return m_alphaMaskTexture;
        }

        std::string m_name;
    private:
        std::array<unsigned int, 5> m_materialTextures;
        std::array<glm::vec3, 5> m_materialColors;

        float m_alpha;
        unsigned int m_alphaMaskTexture;
};

using pbr_material_ptr = std::shared_ptr<pbr_material>;

#endif