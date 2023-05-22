#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <string>

class resource_manager {
    public:
        static auto& getInstance() {
            static resource_manager instance;
            return instance;
        }

        static std::string getAssetPath() {
            return "./../../data/";
        }

        unsigned int loadTexture(std::string path, const bool useMipMaps = true) const;
        unsigned int loadHDRI(const std::string path) const;

        std::string loadTextFile(const std::string path) const;
};

#endif