#include "pbr_material.h"

#include "../utility/resource_manager.h"

pbr_material::pbr_material() {
    // Set material defaults
    std::fill(m_materialTextures.begin(), m_materialTextures.end(), 0);
    std::fill(m_materialColors.begin(), m_materialColors.end(), glm::vec3(0.0f));
}

void pbr_material::init(const std::string name,
                    const std::string albedoPath,
                    const std::string aoPath,
                    const std::string metallicPath,
                    const std::string normalPath,
                    const std::string roughnessPath,
                    const std::string alphaMaskPath)
{
    m_name = name;

    m_materialTextures[ALBEDO] = resource_manager::get_instance().load_texture(albedoPath);
	m_materialTextures[AO] = resource_manager::get_instance().load_texture(aoPath);
	m_materialTextures[METALLIC] = resource_manager::get_instance().load_texture(metallicPath);
	m_materialTextures[NORMAL] = resource_manager::get_instance().load_texture(normalPath);
	m_materialTextures[ROUGHNESS] = resource_manager::get_instance().load_texture(roughnessPath);
	
	m_alpha = resource_manager::get_instance().load_texture(alphaMaskPath);
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

	m_materialColors[ALBEDO] = albedo;
	m_materialColors[AO] = ao;
	m_materialColors[METALLIC] = metallic;
	m_materialColors[NORMAL] = normal;
	m_materialColors[ROUGHNESS] = roughness;

	m_alpha = alpha;
}

unsigned int pbr_material::get_parameter_texture(const ParameterType parameter) const {
	return m_materialTextures[parameter];
}

glm::vec3 pbr_material::get_parameter_color(const ParameterType parameter) const {
	return m_materialColors[parameter];
}