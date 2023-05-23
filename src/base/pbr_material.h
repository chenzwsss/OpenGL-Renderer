#ifndef PBR_MATERIAL_H
#define PBR_MATERIAL_H

#include <glm/vec3.hpp>

#include <memory>
#include <array>
#include <string>

class pbr_material {
    public:
        pbr_material();

        enum ParameterType : int {
            ALBEDO = 0,
            AO,
            METALLIC,
            NORMAL,
            ROUGHNESS
        };
};

#endif