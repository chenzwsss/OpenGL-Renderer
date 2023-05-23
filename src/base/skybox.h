#ifndef SKYBOX_H
#define SKYBOX_H

#include <string>

#include "../graphic/gl_vertex_array.h"

class skybox {
    public:
        void init(const std::string hdrPath, const GLsizei resolution = 512);
        void draw();

        auto get_irradiance_map() const { return m_irradianceMap; }
        auto get_prefilter_map() const { return m_prefilterMap; }
        auto get_brdf_lut() const { return m_brdfLUT; }
    private:
        void render_cube();
        unsigned int m_cubeVAO, m_envCubemap, m_envMapFBO, m_irradianceMap, m_prefilterMap, m_brdfLUT;
        gl_vertex_array m_quadVAO;
};

#endif