
// =======================================================================
// File: src/Shader.cpp
// =======================================================================
#include "Shader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSource = loadShaderSource(vertexPath);
    std::string fragmentSource = loadShaderSource(fragmentPath);
    m_RendererID = createShaderProgram(vertexSource, fragmentSource);
}
Shader::~Shader() { glDeleteProgram(m_RendererID); }
void Shader::Bind() const { glUseProgram(m_RendererID); }
void Shader::Unbind() const { glUseProgram(0); }
void Shader::SetUniform1i(const std::string& name, int value) { glUniform1i(getUniformLocation(name), value); }
void Shader::SetUniform1f(const std::string& name, float value) { glUniform1f(getUniformLocation(name), value); }
void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2) { glUniform3f(getUniformLocation(name), v0, v1, v2); }
void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) { glUniform4f(getUniformLocation(name), v0, v1, v2, v3); }
void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix) { glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix)); }
GLint Shader::getUniformLocation(const std::string& name) {
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end()) { return m_UniformLocationCache[name]; }
    GLint location = glGetUniformLocation(m_RendererID, name.c_str());
    if (location == -1) { std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl; }
    m_UniformLocationCache[name] = location;
    return location;
}
std::string Shader::loadShaderSource(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) { std::cerr << "Error: Could not open shader file: " << filepath << std::endl; return ""; }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
GLuint Shader::compileShader(GLuint type, const std::string& source) {
    GLuint id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> message(length);
        glGetShaderInfoLog(id, length, &length, message.data());
        std::cerr << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cerr << message.data() << std::endl;
        glDeleteShader(id);
        return 0;
    }
    return id;
}
GLuint Shader::createShaderProgram(const std::string& vertexShader, const std::string& fragmentShader) {
    GLuint program = glCreateProgram();
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShader);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);
    if (vs == 0 || fs == 0) return 0;
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

