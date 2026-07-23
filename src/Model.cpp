#include "Model.h"
#include <filesystem>
#include <unordered_map>
#include <iostream>

/**
 * @brief constructor
 * @param path path naar file
 */
Model::Model(const std::string& path) {
    loadModel(path);
}

/**
 * @brief destructor
 */
Model::~Model() {
    for (auto& tex : textures_loaded) {
        if (tex.id != 0) {
            glDeleteTextures(1, &tex.id);
        }
    }
    textures_loaded.clear();
    meshes.clear();
}

/**
 * @brief tekent alle meshes van het model
 * @param shader shader die gebruikt moet worden voor rendering
 */
void Model::Draw(Shader& shader) {
    for (auto& mesh : meshes)
        mesh.Draw(shader);
}

 /**
 * @brief laadt het modelbestand via Assimp
 * @param path path naar file
 */
void Model::loadModel(const std::string& path) {
    _scene = importer.ReadFile(path,
        aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!_scene || _scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !_scene->mRootNode) {
        std::cerr << "Assimp fout: " << importer.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));
    processNode(_scene->mRootNode, _scene);
}

/**
 * @brief verwerkt recursief Assimp node en alle sub nodes en meshes
 * @param node Assimp node
 * @param scene Assimp scene
 */

void Model::processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
        meshes.push_back(processMesh(scene->mMeshes[node->mMeshes[i]], scene));
    for (unsigned int i = 0; i < node->mNumChildren; i++)
        processNode(node->mChildren[i], scene);
}

/**
 * @brief converteert Assimp mesh naar Mesh class
 * @param mesh Assimp mesh
 * @param scene Assimp scene
 * @return Mesh generated Mesh object
 */
Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex v;
        v.Position  = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        if (mesh->HasNormals()) {
            v.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
        } else {
            v.Normal = { 0.0f, 1.0f, 0.0f };
        }
        v.TexCoords = mesh->mTextureCoords[0] ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : glm::vec2(0.0f);
        vertices.push_back(v);
    }
    
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
            indices.push_back(mesh->mFaces[i].mIndices[j]);

    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        auto diff = loadMaterialTextures(mat, aiTextureType_DIFFUSE, "texture_diffuse");
        auto norm = loadMaterialTextures(mat, aiTextureType_NORMALS, "texture_normal");
        textures.insert(textures.end(), diff.begin(), diff.end());
        textures.insert(textures.end(), norm.begin(), norm.end());
    }

    return Mesh(vertices, indices, textures);
}

/**
 * @brief laadt materiaaltexturen
 * @param mat pointer naar Assimp materiaal
 * @param type type textuur
 * @param typeName naam die in de shader gebruikt wordt
 * @return std::vector<Texture> lijst met geladen Texture structs
 */
std::vector<Texture> Model::loadMaterialTextures(
    aiMaterial* mat, aiTextureType type, const std::string& typeName)
{
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);

        bool skip = false;
        for (auto& t : textures_loaded) {
            if (t.path == str.C_Str()) { 
                textures.push_back(t); 
                skip = true; 
                break; 
            }
        }

        if (!skip) {
            Texture tex;

            // checken op embedded textures
            if (str.C_Str()[0] == '*') {
                int index = std::atoi(str.C_Str() + 1);
                tex.id = textureFromEmbedded(_scene, index);  
            } else {
                tex.id = textureFromFile(directory + "/" + str.C_Str());
            }

            tex.type = typeName;
            tex.path = str.C_Str();
            textures.push_back(tex);
            textures_loaded.push_back(tex);
        }
    }
    return textures;
}


/**
 * @brief laadt een embedded textuur uit het 3D-model file
 * @param scene pointer naar Assimp scene
 * @param index index van embedded textuur
 * @return unsigned int generated OpenGL texture ID
 */
unsigned int Model::textureFromEmbedded(const aiScene* scene, int index) {
    const aiTexture* tex = scene->mTextures[index];
    unsigned int id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    int w, h, n;
    unsigned char* data = nullptr;

    if (tex->mHeight == 0) {
        // Gecomprimeerde textuur (png/jpg ingebakken)
        data = stbi_load_from_memory(
            reinterpret_cast<unsigned char*>(tex->pcData),
            tex->mWidth, &w, &h, &n, 0);
    } else {
        // Ruwe RGBA data
        w = tex->mWidth;
        h = tex->mHeight;
        data = reinterpret_cast<unsigned char*>(tex->pcData);
        n = 4;
    }

    if (data) {
        GLenum fmt = (n == 1) ? GL_RED : (n == 3) ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if (tex->mHeight == 0) stbi_image_free(data);
        std::cout << "Embedded textuur geladen: index " << index << std::endl;
    } else {
        std::cerr << "Embedded textuur mislukt: index " << index << std::endl;
    }

    return id;
}


/**
 * @brief laadt externe textuur van file
 * @param filename path naar image
 * @return unsigned int generated OpenGL texture ID
 */
unsigned int Model::textureFromFile(const std::string& filename) {
    unsigned int id;
    glGenTextures(1, &id);
    int w, h, n;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filename.c_str(), &w, &h, &n, 0);
    if (data) {
        GLenum fmt = (n == 1) ? GL_RED : (n == 3) ? GL_RGB : GL_RGBA;
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        std::cerr << "Texture niet gevonden: " << filename << std::endl;
    }
    stbi_image_free(data);
    return id;
}