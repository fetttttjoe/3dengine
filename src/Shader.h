
// =======================================================================
// File: src/Shader.h
// Description: A self-contained class that encapsulates all logic for an
//              OpenGL shader program.
// =======================================================================
#pragma once
#include <string>
#include <unordered_map>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader {
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void Bind() const;
    void Unbind() const;

    void SetUniform1i(const std::string& name, int value);
    void SetUniform1f(const std::string& name, float value);
    void SetUniform3f(const std::string& name, float v0, float v1, float v2);
    void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
    void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);

private:
    std::string loadShaderSource(const std::string& filepath);
    GLuint compileShader(GLuint type, const std::string& source);
    GLuint createShaderProgram(const std::string& vertexShader, const std::string& fragmentShader);
    GLint getUniformLocation(const std::string& name);

    GLuint m_RendererID = 0;
    std::unordered_map<std::string, GLint> m_UniformLocationCache;
};
