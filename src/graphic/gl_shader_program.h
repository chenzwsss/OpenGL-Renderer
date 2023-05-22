#ifndef GL_SHADER_PROGRAM_H
#define GL_SHADER_PROGRAM_H

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>

#include <unordered_map>
#include <string>

#include "shader_create_info.h"

class gl_shader_program {
    public:
        gl_shader_program(const std::string programName, const std::vector<shader_create_info> stages);
        ~gl_shader_program();

        void bind() const;
        void deleteProgram() const;

        void setUniformi(const std::string& uniformName, const int value);
        void setUniformf(const std::string& uniformName, const float value);
        void setUniform(const std::string& uniformName, const glm::ivec2& value);
        void setUniform(const std::string& uniformName, const glm::vec2& value);
        void setUniform(const std::string& uniformName, const glm::vec3& value);
        void setUniform(const std::string& uniformName, const glm::vec4& value);
        void setUniform(const std::string& uniformName, const glm::mat3x3& value);
        void setUniform(const std::string& uniformName, const glm::mat4x4& value);

        auto getProgramName() const { return m_programName; }

    private:
        void collectUniforms();

        std::unordered_map<std::string, int> m_uniforms;

        GLuint m_programID { 0 };
        std::string m_programName;
};

#endif