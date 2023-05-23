#ifndef MODEL_H
#define MODEL_H

#include <memory>
#include <string>

#include "mesh.h"

struct aiScene;
struct aiNode;
struct aiMesh;

class model {
    public:

    private:
        void process_node(aiNode* node, const aiScene* scene, const bool loadMaterial);
};

#endif