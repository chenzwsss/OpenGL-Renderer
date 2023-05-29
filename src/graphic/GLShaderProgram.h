#ifndef GL_SHADER_PROGRAM_H
#define GL_SHADER_PROGRAM_H

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>

#include <unordered_map>
#include <string>

#include "ShaderCreateInfo.h"

class GLShaderProgram {
    public:
        GLShaderProgram(const std::string program_name, const std::vector<ShaderCreateInfo> stages);
        ~GLShaderProgram();

        void bind() const;
        void deleteProgram() const;

        void setUniformi(const std::string& uniform_name, const int value);
        void setUniformf(const std::string& uniform_name, const float value);
        void setUniform(const std::string& uniform_name, const glm::ivec2& value);
        void setUniform(const std::string& uniform_name, const glm::vec2& value);
        void setUniform(const std::string& uniform_name, const glm::vec3& value);
        void setUniform(const std::string& uniform_name, const glm::vec4& value);
        void setUniform(const std::string& uniform_name, const glm::mat3x3& value);
        void setUniform(const std::string& uniform_name, const glm::mat4x4& value);

    private:
        GLuint getUniformLocation(const std::string& uniform_name);

        GLuint m_programId { 0 };
        std::string m_programName;
};

#endif