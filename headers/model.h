#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb_image.h>
#include "mesh.h"
#include "shader.h"
#include <vector>
#include <string>
#include <iostream>

class Model {
public:
    Model(const std::string& path) { loadModel(path); }
    void Draw(Shader& shader);
    void Delete();

private:
    Assimp::Importer  importer;       // member! anders wordt _scene ongeldig
    const aiScene*    _scene = nullptr;

    std::vector<Mesh>          meshes;
    std::vector<Mesh::Texture> textures_loaded;
    std::string                directory;

    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);

    std::vector<Mesh::Texture> loadMaterialTextures(
        aiMaterial* mat, aiTextureType type, const std::string& typeName);

    unsigned int textureFromFile(const std::string& filename);
    unsigned int textureFromEmbedded(const aiScene* scene, int index);  // geen Model::
};