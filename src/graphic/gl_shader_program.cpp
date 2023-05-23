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

void scanForIncludes(std::string& shaderCode) {
    std::size_t startPos = 0;
    const static std::string includeDirective{ "#include " };

    // Scan string for all instances of include directive
    while ((startPos = shaderCode.find(includeDirective, startPos)) != std::string::npos) {
        // Find position of include directive
        const auto pos = startPos + includeDirective.length() + 1;
        const auto length = shaderCode.find('"', pos);
        const auto pathToIncludedFile = shaderCode.substr(pos, length - pos);

        // Load included file
        const auto includedFile = resource_manager::getInstance().loadTextFile(pathToIncludedFile) + "\n";
        // Insert into shader code
        shaderCode.replace(startPos, (length + 1) - startPos, includedFile);
        
        // Increment start position and continue scanning
        startPos += includedFile.length();
    }
}

void compile(const GLuint id, const GLchar* shaderCode) {
    glShaderSource(id, 1, &shaderCode, nullptr);
    glCompileShader(id);
}

bool compileStage(const GLuint id, const std::string shaderType, const std::string& shaderCode) {
    GLint success{ GL_FALSE };
    GLint logLength{ -1 };

    compile(id, shaderCode.c_str());

    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE) {
        GLint infoLogLen = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLen);
        std::vector<GLchar> infoLog(infoLogLen);
        glGetShaderInfoLog(id, infoLogLen, nullptr, infoLog.data());
        std::cerr << "Failed to compile shader. Info log:\n" << infoLog.data() << std::endl;
    }

    return success == GL_TRUE;
}

bool linkProgram(const GLuint id) {
    GLint success{ GL_FALSE };
    GLint logLength{ -1 };
    
    glLinkProgram(id);

    glGetProgramiv(id, GL_LINK_STATUS, &success);

    if (success == GL_FALSE) {
        GLint infoLogLen = 0;
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLen);
        std::vector<GLchar> infoLog(infoLogLen);
        glGetProgramInfoLog(id, infoLogLen, nullptr, infoLog.data());
        std::cerr << "Failed to link shader program. Info log:\n" << infoLog.data() << std::endl;
    }

    return success == GL_TRUE;
}

gl_shader_program::gl_shader_program(const std::string programName, const std::vector<shader_create_info> stages)
    : m_programName(programName) {

    std::cout << "Building shader program " << programName << std::endl;

    std::vector<GLuint> shaderIds;

    bool success { true };
    for (auto i = 0; i < stages.size(); ++i) {
        auto id{ glCreateShader(GL_SHADER_TYPE_ENUM.at(stages[i].type)) };
        shaderIds.push_back(id);

        auto shaderCode{ resource_manager::getInstance().loadTextFile(stages[i].filePath) };
        scanForIncludes(shaderCode);

        if (!compileStage(id, stages[i].type, shaderCode)) {
            success = false;
            break;
        }
    }

    if (!success) {
        for (const auto id : shaderIds) {
            glDeleteShader(id);
        }
        std::cout << "Create shaders failed!" << std::endl;
        return;
    }

    m_programID = glCreateProgram();

    for (const auto id : shaderIds) {
        glAttachShader(m_programID, id);
    }

    if (!linkProgram(m_programID)) {
        for (const auto id : shaderIds) {
            glDetachShader(m_programID, id);
            glDeleteShader(id);
        }
        glDeleteProgram(m_programID);
        std::cout << "Create shader program failed!" << std::endl;
        return;
    }

    collectUniforms();
}

gl_shader_program::~gl_shader_program() {
    deleteProgram();
}

void gl_shader_program::bind() const {
    assert(m_programID != 0);

    glUseProgram(m_programID);
}

void gl_shader_program::deleteProgram() const {
    if (m_programID != 0) {
        std::cout << "Deleting program: " << m_programName << '\n';
        glDeleteProgram(m_programID);
    }
}

void gl_shader_program::setUniformi(const std::string& uniformName, const int value) {
    glUniform1i(m_uniforms.at(uniformName), value);
}

void gl_shader_program::setUniformf(const std::string& uniformName, const float value) {
    glUniform1f(m_uniforms.at(uniformName), value);
}

void gl_shader_program::setUniform(const std::string& uniformName, const glm::ivec2& value) {
    glUniform2iv(m_uniforms.at(uniformName), 1, &value[0]);
}

void gl_shader_program::setUniform(const std::string& uniformName, const glm::vec2& value) {
    glUniform2f(m_uniforms.at(uniformName), value.x, value.y);
}

void gl_shader_program::setUniform(const std::string& uniformName, const glm::vec3& value) {
    glUniform3f(m_uniforms.at(uniformName), value.x, value.y, value.z);
}

void gl_shader_program::setUniform(const std::string& uniformName, const glm::vec4& value) {
    glUniform4f(m_uniforms.at(uniformName), value.x, value.y, value.z, value.w);
}

void gl_shader_program::setUniform(const std::string& uniformName, const glm::mat3x3& value) {
    glUniformMatrix3fv(m_uniforms.at(uniformName), 1, GL_FALSE, value_ptr(value));
}

void gl_shader_program::setUniform(const std::string& uniformName, const glm::mat4x4& value) {
    glUniformMatrix4fv(m_uniforms.at(uniformName), 1, GL_FALSE, value_ptr(value));
}

void gl_shader_program::collectUniforms() {
    int total = -1;
    glGetProgramiv(m_programID, GL_ACTIVE_UNIFORMS, &total);

    for (auto i = 0; i < total; ++i) {
        auto name_len = -1, num = -1;
        GLenum type = GL_ZERO;
        char name[100];
        glGetActiveUniform(m_programID, static_cast<GLuint>(i), sizeof(name) - 1, &name_len, &num, &type, name);
        name[name_len] = 0;

        const auto nameStr = std::string(name);

        m_uniforms.try_emplace(nameStr, glGetUniformLocation(m_programID, name));
    }
}