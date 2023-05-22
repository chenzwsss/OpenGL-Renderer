#ifndef SHADER_CREATE_INFO_H
#define SHADER_CREATE_INFO_H

#include <string>

struct shader_create_info {
    shader_create_info() = default;
    shader_create_info(const std::string path, const std::string type)
        : filePath(path), type(type) {}

    std::string filePath;
    std::string type;
};


#endif