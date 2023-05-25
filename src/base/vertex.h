#ifndef VERTEX_H
#define VERTEX_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct vertex {
    using vec2 = glm::vec2;
    using vec3 = glm::vec3;

    vertex() = default;

    vertex(const vec3& position, const vec2& texcoords)
        : Position(position), TexCoords(texcoords), Normal(vec3(0.0f)), Tangent(vec3(0.0f)), Bitangent(vec3(0.0f))
    {}

    vertex(const vec3& position, const vec2& texcoords, const vec3& normal)
        : Position(position), TexCoords(texcoords), Normal(normal), Tangent(vec3(0.0f)), Bitangent(vec3(0.0f))
    {}

    vertex(const vec3& position, const vec2& texcoords, const vec3& normal, const vec3& tangent)
        : Position(position), TexCoords(texcoords), Normal(normal), Tangent(tangent), Bitangent(vec3(0.0f))
    {}

    vertex(const vec3& position, const vec2& texcoords, const vec3& normal, const vec3& tangent, const vec3& bitangent)
        : Position(position), TexCoords(texcoords), Normal(normal), Tangent(tangent), Bitangent(bitangent)
    {}

    vec3 Position;
    vec3 Normal;
    vec2 TexCoords;
    vec3 Tangent;
    vec3 Bitangent;
};


#endif