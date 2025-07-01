#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp> // For glm::mat4
#include <cstdint>     // For uint32_t

class Shader {
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    void Bind() const;
    void Unbind() const;

    // Uniform setters
    void SetUniform1ui(const std::string& name, uint32_t value);
    void SetUniform1i(const std::string& name, int value);   
    void SetUniform1f(const std::string& name, float value);     
    void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
    void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);

private:
    std::string loadShaderSource(const std::string& filepath);
    unsigned int compileShader(unsigned int type, const std::string& source);
    unsigned int createShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);
    int getUniformLocation(const std::string& name);

    unsigned int m_RendererID;
    std::unordered_map<std::string, int> m_UniformLocationCache;
};