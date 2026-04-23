#include "model.h"
#include <filesystem>

void Model::Draw(Shader& shader) {
    for (auto& mesh : meshes)
        mesh.Draw(shader);
}

void Model::Delete() {
    for (auto& mesh : meshes)
        mesh.DeleteModel();
}

void Model::loadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp fout: " << importer.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
        meshes.push_back(processMesh(scene->mMeshes[node->mMeshes[i]], scene));
    for (unsigned int i = 0; i < node->mNumChildren; i++)
        processNode(node->mChildren[i], scene);
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Mesh::Vertex>       vertices;
    std::vector<unsigned int>       indices;
    std::vector<Mesh::Texture>      textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Mesh::Vertex v;
        v.Position  = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        if (mesh->HasNormals()) {
            v.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
        } else {
            v.Normal = { 0.0f, 1.0f, 0.0f };
        }
        v.TexCoords = mesh->mTextureCoords[0]
            ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y)
            : glm::vec2(0.0f);
        vertices.push_back(v);
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
            indices.push_back(mesh->mFaces[i].mIndices[j]);

    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        auto diff = loadMaterialTextures(mat, aiTextureType_DIFFUSE,  "texture_diffuse");
        auto norm = loadMaterialTextures(mat, aiTextureType_NORMALS,  "texture_normal");
        textures.insert(textures.end(), diff.begin(), diff.end());
        textures.insert(textures.end(), norm.begin(), norm.end());
    }

    // Fallback when OBJ has no .mtl (or no referenced textures).
    if (textures.empty()) {
        const std::vector<std::pair<std::string, std::string>> fallbackFiles = {
            {"Concrete_016_baseColor.png", "texture_diffuse"},
            {"Concrete_016_normal.png", "texture_normal"}
        };
        for (const auto& [filename, typeName] : fallbackFiles) {
            const std::string fullPath = directory + "/" + filename;
            if (!std::filesystem::exists(std::filesystem::path(fullPath))) {
                continue;
            }

            bool alreadyLoaded = false;
            for (const auto& cached : textures_loaded) {
                if (cached.path == fullPath) {
                    textures.push_back(cached);
                    alreadyLoaded = true;
                    break;
                }
            }
            if (alreadyLoaded) {
                continue;
            }

            Mesh::Texture tex;
            tex.id = textureFromFile(fullPath);
            tex.type = typeName;
            tex.path = fullPath;
            textures.push_back(tex);
            textures_loaded.push_back(tex);
        }
    }

    return Mesh(vertices, indices, textures);
}

std::vector<Mesh::Texture> Model::loadMaterialTextures(
    aiMaterial* mat, aiTextureType type, const std::string& typeName)
{
    std::vector<Mesh::Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        // check cache
        bool skip = false;
        for (auto& t : textures_loaded)
            if (t.path == str.C_Str()) { textures.push_back(t); skip = true; break; }
        if (!skip) {
            Mesh::Texture tex;
            tex.id   = textureFromFile(directory + "/" + str.C_Str());
            tex.type = typeName;
            tex.path = str.C_Str();
            textures.push_back(tex);
            textures_loaded.push_back(tex);
        }
    }
    return textures;
}

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