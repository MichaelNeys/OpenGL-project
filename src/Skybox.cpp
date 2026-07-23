#include "Skybox.h"
#include "Shader.h"
#include "stb_image.h"
#include "GeometryData.h"
#include <iostream>

/**
 * @brief constructor
 * 
 * init VAO en VBO met GeometryData
 * init cubemap texture en shader
 * 
 * @param faces paths naar files
 * @param vertPath path naar vert shader
 * @param fragPath path naar frag shader
 */
Skybox::Skybox(const std::vector<std::string>& faces,
               const std::string& vertPath,
               const std::string& fragPath)
{
    // VAO/VBO opzetten
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Geometry::skyboxVertices), Geometry::skyboxVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    cubemapTexture = loadCubemap(faces);
    shader = new Shader(vertPath.c_str(), fragPath.c_str());
}

/**
 * @brief destructor
 * 
 * clear openGL buffers en shader object
 */
Skybox::~Skybox() {
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    if (cubemapTexture != 0) {
        glDeleteTextures(1, &cubemapTexture);
        cubemapTexture = 0;
    }
    if (shader != nullptr) {
        delete shader;
        shader = nullptr;
    }
}

/**
 * @brief laadt 6 afbeeldingen als 1 OpenGL Cubemap Texture `GL_TEXTURE_CUBE_MAP`
 * 
 * @param faces paths naar 6 zijden vd kubus
 * @return unsigned int generated OpenGL Texture ID vd cubemap
 */
unsigned int Skybox::loadCubemap(const std::vector<std::string>& faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        std::cout << "Loading: " << faces[i] << std::endl;
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            std::cout << "  OK " << width << "x" << height << std::endl;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cerr << "Cubemap tex failed: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    // parameters voor overgangen
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

/**
 * @brief rendert skybox als achetergrond vd scene
 * 
 * skybox renderen achter alle andere objecten door depth test en culling uit te schakelen
 * remove translatie uit view matrix zodat skybox niet mee beweegt met camera
 * bind VAO en cubemap texture
 * standard OpenGL herstellen
 * 
 * @param view 4x4 View Matrix vd camera
 * @param projection 4x4 Projection Matrix
 */
void Skybox::Draw(const glm::mat4& view, const glm::mat4& projection) {
    if (!shader) return;

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    shader->use();
    
    glm::mat4 skyView = glm::mat4(glm::mat3(view));
    shader->setMat4("view", skyView);
    shader->setMat4("projection", projection);

    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    shader->setInt("skybox", 0);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}