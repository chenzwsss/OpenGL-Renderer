#ifndef SHADER_CREATE_INFO_H
#define SHADER_CREATE_INFO_H

#include <string>

struct ShaderCreateInfo {
    ShaderCreateInfo() = default;
    ShaderCreateInfo(const std::string path, const std::string type)
        : filePath(path), type(type) {}

    std::string filePath;
    std::string type;
};


#endif