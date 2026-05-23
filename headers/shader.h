#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <string>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
    unsigned int ID;

    // constructor: leest .vert en .frag en compileert ze
    Shader(const char* vertexPath, const char* fragmentPath);

    // activeert shader programma
    void use();

    // hulpfuncties
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setFloat4(const std::string &name, float x, float y, float z, float w) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;

private:
    std::string vertPath, fragPath;
};

#endif