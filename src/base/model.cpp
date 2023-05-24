#include "model.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <iostream>

#include "../utility/resource_manager.h"

model::model(const std::string path, const std::string name, const bool flip_winding_order, const bool load_material)
    : m_scale(glm::vec3(1.0f)), m_position(glm::vec3(0.0f)), m_axis(glm::vec3(0.0f, 1.0f, 0.0f)), m_degrees(0.0f), m_num_mats(0)
{
    if (!load_model(path, flip_winding_order, load_material)) {
        std::cerr << "Failed to load model: " << name << '\n';
    }
}

model::model(const std::string name, const std::vector<vertex>& vertices, const std::vector<GLuint>& indices, const pbr_material_ptr& material)
    : m_scale(glm::vec3(1.0f)), m_position(glm::vec3(0.0f)), m_axis(glm::vec3(0.0f, 1.0f, 0.0f)), m_degrees(0.0f), m_num_mats(0)
{
    m_meshes.emplace_back(vertices, indices, material);
}

model::model(const std::string name, const base_mesh& mesh)
    : m_scale(glm::vec3(1.0f)), m_position(glm::vec3(0.0f)), m_axis(glm::vec3(0.0f, 1.0f, 0.0f)), m_degrees(0.0f), m_num_mats(0)
{
    m_meshes.push_back(mesh);
}

void model::attach_mesh(const base_mesh mesh) {
    m_meshes.push_back(mesh);
}

void model::scale(const glm::vec3& scale) {
    m_scale = scale;
}

void model::rotate(const float degrees, const glm::vec3& axis) {
    m_degrees = degrees;
    m_axis = axis;
}

void model::translate(const glm::vec3& pos) {
    m_position = pos;
}

glm::mat4 model::get_model_matrix() const {
    const auto scale = glm::scale(glm::mat4(1.0f), m_scale);
    const auto translate = glm::translate(glm::mat4(1.0f), m_position);
    const auto rotate = glm::rotate(glm::mat4(1.0f), glm::radians(m_degrees), m_axis);
    return scale * translate * rotate;
}

void model::draw(gl_shader_program& shader) {

    // Set model matrix
    shader.set_uniform("model", this->get_model_matrix());

    auto& meshes = this->get_meshes();
    for (auto& mesh : meshes) {
        mesh.draw(shader);
    }
}

void model::destroy() {
    for (auto& mesh : m_meshes) {
        mesh.vao.destroy();
    }
}

bool model::load_model(const std::string path, const bool flip_winding_order, const bool load_material = true) {

    std::string new_path = resource_manager::get_instance().get_assets_path() + path;
#ifdef _DEBUG
    std::cout << "Loading model: " << m_name << '\n';
#endif

    std::cout << sizeof(pbr_material) << '\n';

    Assimp::Importer importer;
    const aiScene* scene = nullptr;

    if (flip_winding_order) {
        scene = importer.ReadFile(new_path, aiProcess_Triangulate |
                                  aiProcess_JoinIdenticalVertices |
                                  aiProcess_GenUVCoords |
                                  aiProcess_SortByPType |
                                  aiProcess_RemoveRedundantMaterials |
                                  aiProcess_FindInvalidData |
                                  aiProcess_FlipUVs |
                                  aiProcess_FlipWindingOrder | // Reverse back-face culling
                                  aiProcess_CalcTangentSpace |
                                  aiProcess_OptimizeMeshes |
                                  aiProcess_SplitLargeMeshes);
    } else {
        scene = importer.ReadFile(new_path, aiProcess_Triangulate |
                                  aiProcess_JoinIdenticalVertices |
                                  aiProcess_GenUVCoords |
                                  aiProcess_SortByPType |
                                  aiProcess_RemoveRedundantMaterials |
                                  aiProcess_FindInvalidData |
                                  aiProcess_FlipUVs |
                                  aiProcess_CalcTangentSpace |
                                  aiProcess_GenSmoothNormals |
                                  aiProcess_ImproveCacheLocality |
                                  aiProcess_OptimizeMeshes |
                                  aiProcess_SplitLargeMeshes);
    }

    if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp Error for " << m_name << ": " << importer.GetErrorString() << '\n';
        importer.FreeScene();
        return false;
    }

    m_path = path.substr(0, path.find_last_of('/'));
    m_path += '/';

    process_node(scene->mRootNode, scene, load_material);

    importer.FreeScene();
    return true;
}

void model::process_node(aiNode* node, const aiScene* scene, const bool load_material) {
    // Process all node meshes
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        auto* mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.push_back(process_mesh(mesh, scene, load_material));
    }

    // Process their children via recursive tree traversal
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        process_node(node->mChildren[i], scene, load_material);
    }
}

base_mesh model::process_mesh(aiMesh* mesh, const aiScene* scene, const bool load_material) {
    std::vector<vertex> vertices;

    // vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        vertex v;

        if (mesh->HasPositions()) {
            v.Position.x = mesh->mVertices[i].x;
            v.Position.y = mesh->mVertices[i].y;
            v.Position.z = mesh->mVertices[i].z;
        }
        if (mesh->HasNormals()) {
            v.Normal.x = mesh->mNormals[i].x;
            v.Normal.y = mesh->mNormals[i].y;
            v.Normal.z = mesh->mNormals[i].z;
        }
        if (mesh->HasTangentsAndBitangents()) {
            v.Tangent.x = mesh->mTangents[i].x;
            v.Tangent.y = mesh->mTangents[i].y;
            v.Tangent.z = mesh->mTangents[i].z;
        }
        if (mesh->HasTextureCoords(0) && load_material) {
            v.TexCoords.x = mesh->mTextureCoords[0][i].x;
            v.TexCoords.y = mesh->mTextureCoords[0][i].y;
        } else {
            v.TexCoords = glm::vec2(0.0f);
        }

        vertices.push_back(v);
    }

    // indices
    std::vector<GLuint> indices;
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        const auto face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.emplace_back(face.mIndices[j]);
        }
    }

    // materials
    if (load_material) {
        if (mesh->mMaterialIndex >= 0) {
            const auto* mat = scene->mMaterials[mesh->mMaterialIndex];

            aiString name;
            mat->Get(AI_MATKEY_NAME, name);

            // Get the first texture for each texture type we need
            // since there could be multiple textures per type
            aiString albedo_path;
            mat->GetTexture(aiTextureType_DIFFUSE, 0, &albedo_path);

            aiString metallic_path;
            mat->GetTexture(aiTextureType_AMBIENT, 0, &metallic_path);

            aiString normal_path;
            mat->GetTexture(aiTextureType_HEIGHT, 0, &normal_path);

            aiString roughness_path;
            mat->GetTexture(aiTextureType_SHININESS, 0, &roughness_path);

            aiString alphaMask_path;
            mat->GetTexture(aiTextureType_OPACITY, 0, &alphaMask_path);


            albedo_path = std::strcmp(albedo_path.C_Str(), "") == 0 ? "" : m_path + albedo_path.C_Str();
            metallic_path = std::strcmp(metallic_path.C_Str(), "") == 0 ? "" : m_path + metallic_path.C_Str();
            normal_path = std::strcmp(normal_path.C_Str(), "") == 0 ? "" : m_path + normal_path.C_Str();
            roughness_path = std::strcmp(roughness_path.C_Str(), "") == 0 ? "" : m_path + roughness_path.C_Str();
            alphaMask_path = std::strcmp(alphaMask_path.C_Str(), "") == 0 ? "" : m_path + alphaMask_path.C_Str();

            auto new_mat = pbr_material();
            new_mat.init(name.C_Str(),
                albedo_path.C_Str(),
                "",
                metallic_path.C_Str(),
                normal_path.C_Str(),
                roughness_path.C_Str(),
                alphaMask_path.C_Str()
            );
            ++m_num_mats;

            return base_mesh(vertices, indices, std::make_shared<pbr_material>(new_mat));
        }
    }

    return base_mesh(vertices, indices);
}