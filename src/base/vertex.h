#ifndef VERTEX_H
#define VERTEX_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct vertex {
    using vec2 = glm::vec2;
    using vec3 = glm::vec3;

    vertex() = default;

    vertex(const vec3& position, const vec2& texcoords) : Position(position),
                                                          TexCoords(texcoords) {
    }

    vertex(const vec3& position, const vec2& texcoords, const vec3& normal) : Position(position),
                                                                                       TexCoords(texcoords),
                                                                                       Normal(normal) {
    }

    vertex(const vec3& position, const vec2& texcoords, const vec3& normal, const vec3& tangent) : Position(position),
                                                                                                   TexCoords(texcoords),
                                                                                                   Normal(normal),
                                                                                                   Tangent(tangent) {
    }

    vec3 Position;
    vec2 TexCoords;
    vec3 Normal;
    vec3 Tangent;
};


#endif