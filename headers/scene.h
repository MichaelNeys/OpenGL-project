#ifndef SCENE_H
#define SCENE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <shader.h>
#include <mesh.h>
#include <model.h>
#include <vector>
#include <string>
#include <deque>

class Scene {
public:
    glm::vec3 lightPos;

    Scene();
    void Draw(Shader& lightingShader, Shader& lampShader, glm::mat4& view, glm::mat4& projection, glm::vec3& cameraPos);
    void Delete();

private:
    struct AtomiumPart {
        Model* model;
        bool isSphere;
        std::string filePath;
    };
    struct DronePart {
        Model* model;
        std::string filePath;
    };

    Mesh* lampMesh;

    void setLightUniforms(Shader& shader);

    unsigned int bezierVAO;
    unsigned int bezierVBO;
    unsigned int bezierPointCount;

    std::vector<AtomiumPart> atomiumParts;
    std::vector<DronePart> droneParts;
    std::deque<std::string> pendingDronePaths;
};

#endif