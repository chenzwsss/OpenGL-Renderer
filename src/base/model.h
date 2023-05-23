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
        bool load_model(const std::string path, const bool flip_winding_order, const bool load_material);
        void process_node(aiNode* node, const aiScene* scene, const bool load_material);
        mesh process_mesh(aiMesh* mesh, const aiScene* scene, const bool load_material);
};

#endif