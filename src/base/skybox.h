#ifndef SKYBOX_H
#define SKYBOX_H

#include <string>

#include "../graphic/gl_vertex_array.h"

class skybox {
    public:
        void init(const std::string hdrPath, const GLsizei resolution = 512);
        void draw();

        auto GetIrradianceMap() const { return m_irradianceMap; }
        auto GetPrefilterMap() const { return m_prefilterMap; }
        auto GetBRDFLUT() const { return m_brdfLUT; }
    private:
        void renderCube();
        unsigned int m_cubeVAO, m_envCubemap, m_envMapFBO, m_irradianceMap, m_prefilterMap, m_brdfLUT;
        gl_vertex_array m_quadVAO;
};

#endif