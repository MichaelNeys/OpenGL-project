#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb_image.h>

#include <string>
#include <vector>
#include <iostream>

#include "mesh.h"
#include "shader.h"

/**
 * @class Model
 * @brief Manages 3D-model bestaande uit meerdere meshes en bijbehorende texturen
 */
class Model {
public:
    Model(const std::string& path);
    ~Model();

    void Draw(Shader& shader);
    
    std::vector<Mesh> meshes;

private:
    Assimp::Importer importer;
    const aiScene* _scene = nullptr;
    
    std::vector<Texture> textures_loaded;
    std::string directory;
    
    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);

    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);
    unsigned int textureFromEmbedded(const aiScene* scene, int index);
    unsigned int textureFromFile(const std::string& filename);
};