#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

/**
 * @brief read file helper function
 * 
 * @param filePath path to file
 * @return std::string content of file
 */
static std::string readFile(const char* filePath) {
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        file.open(filePath);
        std::stringstream stream;
        stream << file.rdbuf();
        file.close();
        return stream.str();
    }
    catch (const std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << filePath 
                  << " (" << e.what() << ")" << std::endl;
        return "";
    }
}

/**
 * @brief helper function to check shader compilation and linking errors
 * 
 * @param shaderOrProgram OpenGL handle from shader of program
 * @param type vert, frag of program
 * @param path The file path for clarification in error messages.
 */
static void checkCompileErrors(unsigned int shaderOrProgram, const std::string& type, const std::string& path = "") {
    int success;
    char infoLog[1024];

    if (type != "PROGRAM") {
        glGetShaderiv(shaderOrProgram, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shaderOrProgram, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER::" << type << "::COMPILATION_FAILED\n"
                      << "  Bestand: " << path << "\n"
                      << infoLog << "\n---------------------------------------------------" << std::endl;
        }
    } else {
        glGetProgramiv(shaderOrProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderOrProgram, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                      << infoLog << "\n---------------------------------------------------" << std::endl;
        }
    }
}

/**
 * @brief helper function to compile a shader from source code
 * 
 * @param shaderType OpenGL type vert or frag
 * @param source source code as string
 * @param path file path
 * @return unsigned int OpenGL shader ID
 */
static unsigned int compileShader(GLenum shaderType, const std::string& source, const char* path) {
    unsigned int shader = glCreateShader(shaderType);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    std::string typeStr = (shaderType == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
    checkCompileErrors(shader, typeStr, path);

    return shader;
}

/**
 * @brief constructor to create shader program from vert and frag
 * 
 * read vert and frag
 * compile
 * link to OpenGL program ID
 * 
 * @param vertexPath path to .vert
 * @param fragmentPath path to .frag
 */
Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    this->vertPath = vertexPath;
    this->fragPath = fragmentPath;

    // 1. Bestanden inlezen
    std::string vertexCode = readFile(vertexPath);
    std::string fragmentCode = readFile(fragmentPath);

    if (vertexCode.empty() || fragmentCode.empty()) {
        std::cerr << "ERROR::SHADER::INIT_FAILED: Een of beide shaderbestanden zijn leeg of onleesbaar" << std::endl;
        return;
    }

    // 2. Shaders compileren
    unsigned int vertex = compileShader(GL_VERTEX_SHADER, vertexCode, vertexPath);
    unsigned int fragment = compileShader(GL_FRAGMENT_SHADER, fragmentCode, fragmentPath);

    // 3. Shader Programma linken
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    // 4. shaders opruimen
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

/**
 * @brief activeert shader voor renderpipeline
 */
void Shader::use() {
    glUseProgram(ID);
}

/**
 * @brief zet bool op shader
 * @param name naam van variabele in GLSL
 * @param value bool waarde
 */
void Shader::setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), static_cast<int>(value));
}

/**
 * @brief zet int op shader
 * @param name naam van variabele in GLSL
 * @param value int waarde
 */
void Shader::setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

/**
 * @brief zet float op shader
 * @param name naam van variabele in GLSL
 * @param value float waarde
 */
void Shader::setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

/**
 * @brief zet 4 floats op shader
 * @param name naam van variabele in GLSL
 * @param x X-component
 * @param y Y-component
 * @param z Z-component
 * @param w W-component
 */
void Shader::setFloat4(const std::string &name, float x, float y, float z, float w) const {
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}

/**
 * @brief zet 4x4 transformatiematrix op shader
 * @param name naam van variabele in GLSL
 * @param mat glm::mat4 matrix
 */
void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

/**
 * @brief zet vec3 (3D vector) op shader
 * @param name naam van variabele in GLSL
 * @param value glm::vec3 vector
 */
void Shader::setVec3(const std::string &name, const glm::vec3 &value) const {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}