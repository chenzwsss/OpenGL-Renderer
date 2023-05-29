#ifndef VERTEX_H
#define VERTEX_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct Vertex {
    using vec2 = glm::vec2;
    using vec3 = glm::vec3;

    vec3 Position;
    vec3 Normal;
    vec2 TexCoords;
    //vec3 Tangent;
    //vec3 Bitangent;
};


#endif