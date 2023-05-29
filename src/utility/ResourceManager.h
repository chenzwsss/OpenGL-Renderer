#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <string>

class ResourceManager {
    public:
        static auto& get_instance() {
            static ResourceManager instance;
            return instance;
        }

        static std::string get_assets_path() {
            return "./../../data/";
        }

        unsigned int load_texture(std::string path, const bool useMipMaps = true) const;
        unsigned int load_hdr_i(const std::string path) const;

        unsigned int texture_from_buffer(void* buffer, std::string name, int width, int height, int nrComponents, const bool useMipMaps = true);

        std::string load_text_file(const std::string path) const;
};

#endif