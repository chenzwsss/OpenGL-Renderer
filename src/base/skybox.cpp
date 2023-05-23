#include "skybox.h"

#include <array>

#include <glm/gtc/matrix_transform.hpp>

#include "../utility/resource_manager.h"
#include "../graphic/gl_shader_program.h"

const std::array<float, 108> vertices{
    // back face
    -1.0f, -1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    // front face
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
    // left face
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    // right face
    1.0f,  1.0f,  1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f, -1.0f,  1.0f,
    // bottom face
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f,  1.0f,
    1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    // top face
    -1.0f,  1.0f, -1.0f,
    1.0f,  1.0f , 1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f
};

void skybox::init(const std::string hdrPath, const GLsizei resolution) {
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // No seams at cubemap edges

    glGenVertexArrays(1, &m_cubeVAO);
    glBindVertexArray(m_cubeVAO);

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    // 1.Environment map FBO
    glGenFramebuffers(1, &m_envMapFBO);
    unsigned int envMapRBO;
    glGenRenderbuffers(1, &envMapRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, m_envMapFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, envMapRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, resolution, resolution);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, envMapRBO);

    const auto hdrTexture = resource_manager::get_instance().load_hdr_i(hdrPath);

    glGenTextures(1, &m_envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
    for (auto i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, resolution, resolution, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (helps against bright dot artifacts)
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Setup projection and view matrices for capturing data onto the 6 cubemap face directions
    const auto& captureProjection{ glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f) };
    const glm::mat4 captureViews[] {
        glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    gl_shader_program convertToCubemapShader{ "Equirectangular to Cubemap Shader", {
        {"shaders/cubemapvs.glsl", "vertex"},
        {"shaders/cubemapconverterps.glsl", "fragment"}
    } };

    convertToCubemapShader.bind();
    convertToCubemapShader.set_uniform_i("equirectangularMap", 0);
    convertToCubemapShader.set_uniform("projection", captureProjection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    glViewport(0, 0, resolution, resolution);
    glBindFramebuffer(GL_FRAMEBUFFER, m_envMapFBO);
    for (auto i = 0; i < 6; ++i) {
        convertToCubemapShader.set_uniform("view", captureViews[i]);

        // Attach environment texture to FBO
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render_cube();
    }

    glDeleteTextures(1, &hdrTexture);
    convertToCubemapShader.delete_program();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Generate mipmaps from first mip face (again to reduce bright dots)
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // 2.Precompute irradiance cubemap.
    glGenTextures(1, &m_irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceMap);
    for (auto i = 0; i < 6; ++i) {
        // Convoluting a cubemap purposefully scrubs out the fine details so we only need a low-res image (default 32)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, resolution / 16, resolution / 16, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, m_envMapFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, envMapRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, resolution / 16, resolution / 16);

    // Solve diffuse integral by convolution to create an irradiance cubemap
    gl_shader_program irradianceShader{"Irradiance Shader", {
        {"shaders/cubemapvs.glsl", "vertex"},
        {"shaders/irradianceConvolutionps.glsl", "fragment"}
    }};

    irradianceShader.bind();
    irradianceShader.set_uniform_i("environmentMap", 0);
    irradianceShader.set_uniform("projection", captureProjection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);

    glViewport(0, 0, resolution / 16, resolution / 16);
    glBindFramebuffer(GL_FRAMEBUFFER, m_envMapFBO);
    for (unsigned int i = 0; i < 6; ++i) {
        irradianceShader.set_uniform("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render_cube();
    }
    irradianceShader.delete_program();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 3.Create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale
    glGenTextures(1, &m_prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_prefilterMap);
    for (auto i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, resolution / 4, resolution / 4, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Guess what this is for?? :P
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // Run quasi monte-carlo simulation on the environment lighting to create a prefilter cubemap (since we can't integrate over infinite directions).
    // Pre-filter the environment map with different roughness values over multiple mipmap levels
    gl_shader_program prefilterShader{"Pre-filter Shader", {
        {"shaders/cubemapvs.glsl", "vertex"},
        {"shaders/prefilterps.glsl", "fragment"}
    }};

    prefilterShader.bind();
    prefilterShader.set_uniform_i("environmentMap", 0);
    prefilterShader.set_uniform("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);

    glBindFramebuffer(GL_FRAMEBUFFER, m_envMapFBO);
    const unsigned int maxMipLevels = 5;
    for (unsigned int mipLevel = 0; mipLevel < maxMipLevels; ++mipLevel) {
        // Resize framebuffer according to mip-level size.
        const unsigned int mipWidth = (unsigned int)((resolution / 4) * std::pow(0.5f, mipLevel));
        const unsigned int mipHeight = (unsigned int)((resolution / 4) * std::pow(0.5f, mipLevel));
        glBindRenderbuffer(GL_RENDERBUFFER, envMapRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        const float roughness = static_cast<float>(mipLevel) / static_cast<float>((maxMipLevels - 1));
        prefilterShader.set_uniform_f("roughness", roughness);
        // For 6 cubemap faces
        for (auto i = 0; i < 6; ++i) {
            prefilterShader.set_uniform("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_prefilterMap, mipLevel);
            
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            render_cube();
        }
    }
    prefilterShader.delete_program();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 4.Generate 2D LUT from BRDF equations
    glGenTextures(1, &m_brdfLUT);
    glBindTexture(GL_TEXTURE_2D, m_brdfLUT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, resolution, resolution, 0, GL_RG, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Reconfigure capture framebuffer object and render screen-space quad with BRDF shader
    glBindFramebuffer(GL_FRAMEBUFFER, m_envMapFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, envMapRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, resolution, resolution);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_brdfLUT, 0);

    gl_shader_program brdfShader{"BRDF Shader", {
        {"shaders/brdfvs.glsl", "vertex"},
        {"shaders/brdfps.glsl", "fragment"}
    }};

    brdfShader.bind();
    glViewport(0, 0, resolution, resolution);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_quadVAO.bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    brdfShader.delete_program();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void skybox::draw() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
    render_cube();
}

void skybox::render_cube() {
    glBindVertexArray(m_cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
