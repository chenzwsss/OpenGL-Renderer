#ifndef SKYBOX_H
#define SKYBOX_H

#include <string>

#include "../graphic/GLVertexArray.h"

class Skybox {
    public:
        void init(const std::string hdr_path, const GLsizei resolution = 512);
        void draw();

        auto getIrradianceMap() const { return m_irradianceMap; }
        auto getPrefilterMap() const { return m_prefilterMap; }
        auto getBRDFLUT() const { return m_BRDFLUT; }
    private:
        void renderCube();
        unsigned int m_cubeVAO, m_envCubemap, m_envMapFBO, m_irradianceMap, m_prefilterMap, m_BRDFLUT;
        GLVertexArray m_quadVAO;
};

#endif