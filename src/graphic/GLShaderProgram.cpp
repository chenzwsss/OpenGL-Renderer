#include "GLShaderProgram.h"

#include <iostream>
#include <unordered_map>

#include "../utility/ResourceManager.h"

const std::unordered_map<std::string, int> GL_SHADER_TYPE_ENUM {
    { "vertex", GL_VERTEX_SHADER },
    { "fragment", GL_FRAGMENT_SHADER },
    { "geometry", GL_GEOMETRY_SHADER }
};

void scanForIncludes(std::string& shader_code) {
    std::size_t start_pos = 0;
    const static std::string include_directive{ "#include " };

    // Scan string for all instances of include directive
    while ((start_pos = shader_code.find(include_directive, start_pos)) != std::string::npos) {
        // Find position of include directive
        const auto pos = start_pos + include_directive.length() + 1;
        const auto length = shader_code.find('"', pos);
        const auto path_to_included_file = shader_code.substr(pos, length - pos);

        // Load included file
        const auto included_file = ResourceManager::getInstance().loadTextFile(path_to_included_file) + "\n";
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

bool compileStage(const GLuint id, const ShaderCreateInfo info, const std::string& shader_code) {
    GLint success{ GL_FALSE };

    compile(id, shader_code.c_str());

    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE) {
        GLint info_log_len = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &info_log_len);
        std::vector<GLchar> info_log(info_log_len);
        glGetShaderInfoLog(id, info_log_len, nullptr, info_log.data());
        std::cerr << "Failed to compile shader. Shader path: " << info.filePath << ".\nInfo log:\n" << info_log.data() << std::endl;
    }

    return success == GL_TRUE;
}

bool linkProgram(const GLuint id) {
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

GLShaderProgram::GLShaderProgram(const std::string program_name, const std::vector<ShaderCreateInfo> stages)
    : m_programName(program_name) {

#ifdef _DEBUG
    std::cout << "Building shader program " << program_name << std::endl;
#endif

    std::vector<GLuint> shader_ids;

    bool success { true };
    for (auto i = 0; i < stages.size(); ++i) {
        auto id{ glCreateShader(GL_SHADER_TYPE_ENUM.at(stages[i].type)) };
        shader_ids.push_back(id);

        auto shader_code{ ResourceManager::getInstance().loadTextFile(stages[i].filePath) };
        scanForIncludes(shader_code);

        if (!compileStage(id, stages[i], shader_code)) {
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

    m_programId = glCreateProgram();

    for (const auto id : shader_ids) {
        glAttachShader(m_programId, id);
    }

    if (!linkProgram(m_programId)) {
        for (const auto id : shader_ids) {
            glDetachShader(m_programId, id);
            glDeleteShader(id);
        }
        glDeleteProgram(m_programId);
        std::cout << "Create shader program failed!" << std::endl;
        return;
    }
}

GLShaderProgram::~GLShaderProgram() {
    deleteProgram();
}

void GLShaderProgram::bind() const {
    assert(m_programId != 0);

    glUseProgram(m_programId);
}

void GLShaderProgram::deleteProgram() const {
    if (m_programId != 0) {

#ifdef _DEBUG
    std::cout << "Deleting program: " << m_programName << '\n';
#endif
        glDeleteProgram(m_programId);
    }
}

void GLShaderProgram::setUniformi(const std::string& uniform_name, const int value) {
    glUniform1i(getUniformLocation(uniform_name), value);
}

void GLShaderProgram::setUniformf(const std::string& uniform_name, const float value) {
    glUniform1f(getUniformLocation(uniform_name), value);
}

void GLShaderProgram::setUniform(const std::string& uniform_name, const glm::ivec2& value) {
    glUniform2iv(getUniformLocation(uniform_name), 1, &value[0]);
}

void GLShaderProgram::setUniform(const std::string& uniform_name, const glm::vec2& value) {
    glUniform2f(getUniformLocation(uniform_name), value.x, value.y);
}

void GLShaderProgram::setUniform(const std::string& uniform_name, const glm::vec3& value) {
    glUniform3f(getUniformLocation(uniform_name), value.x, value.y, value.z);
}

void GLShaderProgram::setUniform(const std::string& uniform_name, const glm::vec4& value) {
    glUniform4f(getUniformLocation(uniform_name), value.x, value.y, value.z, value.w);
}

void GLShaderProgram::setUniform(const std::string& uniform_name, const glm::mat3x3& value) {
    glUniformMatrix3fv(getUniformLocation(uniform_name), 1, GL_FALSE, value_ptr(value));
}

void GLShaderProgram::setUniform(const std::string& uniform_name, const glm::mat4x4& value) {
    glUniformMatrix4fv(getUniformLocation(uniform_name), 1, GL_FALSE, value_ptr(value));
}

GLuint GLShaderProgram::getUniformLocation(const std::string& uniform_name) {
    return glGetUniformLocation(m_programId, uniform_name.c_str());
}
