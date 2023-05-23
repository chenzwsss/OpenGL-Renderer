#include "resource_manager.h"

#include <iostream>
#include <sstream>
#include <fstream>

#include <glad/glad.h>

#include <stb_image.h>

unsigned int resource_manager::loadTexture(std::string path, const bool useMipMaps) const {
    if (path.empty())
        return 0;

    std::string new_path = getAssetPath() + path;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width = 0, height = 0, nrComponents = 0;
    unsigned char* data = stbi_load(new_path.c_str(), &width, &height, &nrComponents, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << new_path << std::endl;
        glDeleteTextures(1, &textureID);
        stbi_image_free(data);
        return 0;
    }

    GLenum format = 0;
    switch (nrComponents) {
        case 1:
            format = GL_RED;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
    }

    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (useMipMaps)
        glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    return textureID;
}

unsigned int resource_manager::loadHDRI(const std::string path) const {

    std::string new_path = getAssetPath() + path;

    int width, height, nrComp;
    auto* data{ stbi_loadf(new_path.data(), &width, &height, &nrComp, 0) };

    if (!data) {
        std::cerr << "Resource Manager: Failed to load HDRI." << std::endl;
        std::abort();
    }

    unsigned int hdrTexture{ 0 };
    glGenTextures(1, &hdrTexture);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    return hdrTexture;
}

std::string resource_manager::loadTextFile(const std::string path) const {

    std::string new_path = getAssetPath() + path;

    std::ifstream in(new_path, std::ios::in);
    in.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    if (!in) {
        std::cerr << "Resource Manager: File loading error: " + new_path << " " << errno << std::endl;
        std::abort();
    }

    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}