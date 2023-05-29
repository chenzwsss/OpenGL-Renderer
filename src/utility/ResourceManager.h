#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <string>

class ResourceManager {
    public:
        static auto& getInstance() {
            static ResourceManager instance;
            return instance;
        }

        static std::string getAssetsPath() {
            return "./../../data/";
        }

        unsigned int loadTexture(std::string path, const bool useMipMaps = true) const;
        unsigned int loadHDRI(const std::string path) const;

        unsigned int textureFromBuffer(void* buffer, std::string name, int width, int height, int nrComponents, const bool useMipMaps = true);

        std::string loadTextFile(const std::string path) const;
};

#endif