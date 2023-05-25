#include "gl_shader_program.h"

#include <iostream>
#include <unordered_map>

#include "../utility/resource_manager.h"

const std::unordered_map<std::string, int> GL_SHADER_TYPE_ENUM {
    { "vertex", GL_VERTEX_SHADER },
    { "fragment", GL_FRAGMENT_SHADER },
    { "geometry", GL_GEOMETRY_SHADER },
    { "compute", GL_COMPUTE_SHADER }
};

void scan_for_includes(std::string& shader_code) {
    std::size_t start_pos = 0;
    const static std::string include_directive{ "#include " };

    // Scan string for all instances of include directive
    while ((start_pos = shader_code.find(include_directive, start_pos)) != std::string::npos) {
        // Find position of include directive
        const auto pos = start_pos + include_directive.length() + 1;
        const auto length = shader_code.find('"', pos);
        const auto path_to_included_file = shader_code.substr(pos, length - pos);

        // Load included file
        const auto included_file = resource_manager::get_instance().load_text_file(path_to_included_file) + "\n";
        // Insert into shader code
        shader_code.replace(start_pos, (length + 1) - start_pos, included_file);
        
        // Increment start position and continue scanning
        start_pos += included_file.length();
    }
}

void compile(const GLuint id, const GLchar* shader_code) {
    glShaderSource(id, 1, &shader_code, nullptr);
    glCompileShader(id);
}

bool compile_stage(const GLuint id, const shader_create_info info, const std::string& shader_code) {
    GLint success{ GL_FALSE };

    compile(id, shader_code.c_str());

    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE) {
        GLint info_log_len = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &info_log_len);
        std::vector<GLchar> info_log(info_log_len);
        glGetShaderInfoLog(id, info_log_len, nullptr, info_log.data());
        std::cerr << "Failed to compile shader. Shader path: " << info.file_path << ".\nInfo log:\n" << info_log.data() << std::endl;
    }

    return success == GL_TRUE;
}

bool link_program(const GLuint id) {
    GLint success{ GL_FALSE };
    
    glLinkProgram(id);

    glGetProgramiv(id, GL_LINK_STATUS, &success);

    if (success == GL_FALSE) {
        GLint info_log_len = 0;
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &info_log_len);
        std::vector<GLchar> info_log(info_log_len);
        glGetProgramInfoLog(id, info_log_len, nullptr, info_log.data());
        std::cerr << "Failed to link shader program. Info log:\n" << info_log.data() << std::endl;
    }

    return success == GL_TRUE;
}

gl_shader_program::gl_shader_program(const std::string program_name, const std::vector<shader_create_info> stages)
    : m_program_name(program_name) {

#ifdef _DEBUG
    std::cout << "Building shader program " << program_name << std::endl;
#endif

    std::vector<GLuint> shader_ids;

    bool success { true };
    for (auto i = 0; i < stages.size(); ++i) {
        auto id{ glCreateShader(GL_SHADER_TYPE_ENUM.at(stages[i].type)) };
        shader_ids.push_back(id);

        auto shader_code{ resource_manager::get_instance().load_text_file(stages[i].file_path) };
        scan_for_includes(shader_code);

        if (!compile_stage(id, stages[i], shader_code)) {
            success = false;
            break;
        }
    }

    if (!success) {
        for (const auto id : shader_ids) {
            glDeleteShader(id);
        }
        std::cout << "Create shaders failed!" << std::endl;
        return;
    }

    m_program_id = glCreateProgram();

    for (const auto id : shader_ids) {
        glAttachShader(m_program_id, id);
    }

    if (!link_program(m_program_id)) {
        for (const auto id : shader_ids) {
            glDetachShader(m_program_id, id);
            glDeleteShader(id);
        }
        glDeleteProgram(m_program_id);
        std::cout << "Create shader program failed!" << std::endl;
        return;
    }
}

gl_shader_program::~gl_shader_program() {
    delete_program();
}

void gl_shader_program::bind() const {
    assert(m_program_id != 0);

    glUseProgram(m_program_id);
}

void gl_shader_program::delete_program() const {
    if (m_program_id != 0) {

#ifdef _DEBUG
    std::cout << "Deleting program: " << m_program_name << '\n';
#endif
        glDeleteProgram(m_program_id);
    }
}

void gl_shader_program::set_uniform_i(const std::string& uniform_name, const int value) {
    glUniform1i(get_uniform_location(uniform_name), value);
}

void gl_shader_program::set_uniform_f(const std::string& uniform_name, const float value) {
    glUniform1f(get_uniform_location(uniform_name), value);
}

void gl_shader_program::set_uniform(const std::string& uniform_name, const glm::ivec2& value) {
    glUniform2iv(get_uniform_location(uniform_name), 1, &value[0]);
}

void gl_shader_program::set_uniform(const std::string& uniform_name, const glm::vec2& value) {
    glUniform2f(get_uniform_location(uniform_name), value.x, value.y);
}

void gl_shader_program::set_uniform(const std::string& uniform_name, const glm::vec3& value) {
    glUniform3f(get_uniform_location(uniform_name), value.x, value.y, value.z);
}

void gl_shader_program::set_uniform(const std::string& uniform_name, const glm::vec4& value) {
    glUniform4f(get_uniform_location(uniform_name), value.x, value.y, value.z, value.w);
}

void gl_shader_program::set_uniform(const std::string& uniform_name, const glm::mat3x3& value) {
    glUniformMatrix3fv(get_uniform_location(uniform_name), 1, GL_FALSE, value_ptr(value));
}

void gl_shader_program::set_uniform(const std::string& uniform_name, const glm::mat4x4& value) {
    glUniformMatrix4fv(get_uniform_location(uniform_name), 1, GL_FALSE, value_ptr(value));
}

GLuint gl_shader_program::get_uniform_location(const std::string& uniform_name) {
    return glGetUniformLocation(m_program_id, uniform_name.c_str());
}
