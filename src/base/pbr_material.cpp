#include "pbr_material.h"

#include "../utility/resource_manager.h"

pbr_material::pbr_material() {
    // Set material defaults
    std::fill(m_material_textures.begin(), m_material_textures.end(), 0);
    std::fill(m_material_colors.begin(), m_material_colors.end(), glm::vec3(0.0f));
}

void pbr_material::init(const std::string name,
                    const std::string albedo_path,
                    const std::string ao_path,
                    const std::string metallic_path,
                    const std::string normal_path,
                    const std::string roughness_path,
                    const std::string alpha_mask_path)
{
    m_name = name;

    m_material_textures[ALBEDO] = resource_manager::get_instance().load_texture(albedo_path);
	m_material_textures[AO] = resource_manager::get_instance().load_texture(ao_path);
	m_material_textures[METALLIC] = resource_manager::get_instance().load_texture(metallic_path);
	m_material_textures[NORMAL] = resource_manager::get_instance().load_texture(normal_path);
	m_material_textures[ROUGHNESS] = resource_manager::get_instance().load_texture(roughness_path);

    m_alpha_mask_texture = resource_manager::get_instance().load_texture(alpha_mask_path);
}

void pbr_material::init(const std::string name,
                    const glm::vec3& albedo,
                    const glm::vec3& ao,
                    const glm::vec3& metallic,
                    const glm::vec3& normal,
                    const glm::vec3& roughness,
                    const float alpha)
{
    m_name = name;

	m_material_colors[ALBEDO] = albedo;
	m_material_colors[AO] = ao;
	m_material_colors[METALLIC] = metallic;
	m_material_colors[NORMAL] = normal;
	m_material_colors[ROUGHNESS] = roughness;

	m_alpha = alpha;
}

unsigned int pbr_material::get_parameter_texture(const parameter_type parameter) const {
	return m_material_textures[parameter];
}

glm::vec3 pbr_material::get_parameter_color(const parameter_type parameter) const {
	return m_material_colors[parameter];
}