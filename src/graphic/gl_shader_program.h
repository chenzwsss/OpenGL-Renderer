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
        gl_shader_program(const std::string program_name, const std::vector<shader_create_info> stages);
        ~gl_shader_program();

        void bind() const;
        void delete_program() const;

        void set_uniform_i(const std::string& uniform_name, const int value);
        void set_uniform_f(const std::string& uniform_name, const float value);
        void set_uniform(const std::string& uniform_name, const glm::ivec2& value);
        void set_uniform(const std::string& uniform_name, const glm::vec2& value);
        void set_uniform(const std::string& uniform_name, const glm::vec3& value);
        void set_uniform(const std::string& uniform_name, const glm::vec4& value);
        void set_uniform(const std::string& uniform_name, const glm::mat3x3& value);
        void set_uniform(const std::string& uniform_name, const glm::mat4x4& value);

        auto get_program_name() const { return m_program_name; }

    private:
        void collect_uniforms();

        std::unordered_map<std::string, int> m_uniforms;

        GLuint m_program_id { 0 };
        std::string m_program_name;
};

#endif