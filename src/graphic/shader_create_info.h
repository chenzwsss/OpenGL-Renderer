#ifndef SHADER_CREATE_INFO_H
#define SHADER_CREATE_INFO_H

#include <string>

struct shader_create_info {
    shader_create_info() = default;
    shader_create_info(const std::string path, const std::string type)
        : file_path(path), type(type) {}

    std::string file_path;
    std::string type;
};


#endif