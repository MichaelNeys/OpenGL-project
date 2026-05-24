#include "bluePlane.h"
#include "stb_image.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

static float planeVerts[] = {
    // positie (x,y,z)      texcoords (u,v)
    -0.5f, 0.5f, 0.0f,    0.0f, 0.0f,
    -0.5f, -0.5f, 0.0f,    0.0f, 1.0f,
     0.5f, -0.5f, 0.0f,    1.0f, 1.0f,

    -0.5f, 0.5f, 0.0f,    0.0f, 0.0f,
     0.5f, -0.5f, 0.0f,    1.0f, 1.0f,
     0.5f, 0.5f, 0.0f,    1.0f, 0.0f
};

BluePlane::BluePlane(const char* overlayImagePath, glm::vec3 position, glm::vec2 size)
    : position(position), size(size)
{
    setupPlane();
    planeShader = new Shader("shaders/bluePlane.vert", "shaders/bluePlane.frag");
    overlayTexture = loadTexture(overlayImagePath);
}

void BluePlane::setupPlane() {
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);

    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVerts), planeVerts, GL_STATIC_DRAW);

    // attribuut 0: positie (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // attribuut 1: texcoords (u, v)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

unsigned int BluePlane::loadTexture(const char* path) {
    unsigned int texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, channels;
    stbi_set_flip_vertically_on_load(false);
    unsigned char* data = stbi_load(path, &width, &height, &channels, 0);

    if (data) {
        imageWidth = width;
        imageHeight = height;
        GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        textureLoaded = true;
        std::cout << "BluePlane overlay geladen: " << path << std::endl;
    } else {
        std::cerr << "BluePlane: kon afbeelding niet laden: " << path << std::endl;
        textureLoaded = false;
    }

    stbi_image_free(data);
    return texID;
}

void BluePlane::DrawPlane(glm::mat4& view, glm::mat4& projection) {
    if (!enabled) return;

    float aspectRatio = (imageHeight > 0) ? (float)imageWidth / (float)imageHeight : 1.0f;
    float planeWidth = size.y * aspectRatio;
    float planeHeight = size.y;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(planeWidth, planeHeight, 1.0f));

    planeShader->use();
    planeShader->setMat4("model", model);
    planeShader->setMat4("view", view);
    planeShader->setMat4("projection", projection);
    planeShader->setInt ("overlayTexture", 0);
    planeShader->setBool("showOverlay", showOverlay && textureLoaded);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, overlayTexture);

    glDisable(GL_CULL_FACE);
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glEnable(GL_CULL_FACE);
}

void BluePlane::Delete() {
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteTextures(1, &overlayTexture);
    delete planeShader;
    planeShader = nullptr;
}