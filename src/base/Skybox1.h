#ifndef SKYBOX_H
#define SKYBOX_H

#include <string>

#include "../graphic/gl_vertex_array.h"

class Skybox {
    public:
        void init(const std::string hdr_path, const GLsizei resolution = 512);
        void draw();

        auto get_irradiance_map() const { return m_irradiance_map; }
        auto get_prefilter_map() const { return m_prefilter_map; }
        auto get_brdf_lut() const { return m_brdf_lut; }
    private:
        void render_cube();
        unsigned int m_cube_vao, m_env_cubemap, m_env_map_fbo, m_irradiance_map, m_prefilter_map, m_brdf_lut;
        gl_vertex_array m_quad_vao;
};

#endif