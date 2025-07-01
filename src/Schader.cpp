#include "Shader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
Shader::Shader(const std::string &vp, const std::string &fp)
{
    m_RendererID = createShaderProgram(loadShaderSource(vp), loadShaderSource(fp));
}
Shader::~Shader()
{
    if (m_RendererID != 0)
        glDeleteProgram(m_RendererID);
}
void Shader::Bind() const { glUseProgram(m_RendererID); }
void Shader::Unbind() const { glUseProgram(0); }

// THE FIX: Implementation for the new function.
void Shader::SetUniform1ui(const std::string &name, uint32_t value)
{
    glUniform1ui(getUniformLocation(name), value);
}

void Shader::SetUniform1i(const std::string &name, int value) { glUniform1i(getUniformLocation(name), value); }
void Shader::SetUniform1f(const std::string &name, float value) { glUniform1f(getUniformLocation(name), value); }
void Shader::SetUniform4f(const std::string &name, float v0, float v1, float v2, float v3) { glUniform4f(getUniformLocation(name), v0, v1, v2, v3); }
void Shader::SetUniformMat4f(const std::string &name, const glm::mat4 &matrix) { glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix)); }
int Shader::getUniformLocation(const std::string &name)
{
    if (m_UniformLocationCache.count(name))
        return m_UniformLocationCache[name];
    int loc = glGetUniformLocation(m_RendererID, name.c_str());
    if (loc == -1)
        std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
    m_UniformLocationCache[name] = loc;
    return loc;
}
std::string Shader::loadShaderSource(const std::string &p)
{
    std::ifstream f(p);
    if (!f.is_open())
    {
        return "";
    }
    std::stringstream b;
    b << f.rdbuf();
    return b.str();
}
unsigned int Shader::compileShader(unsigned int t, const std::string &s)
{
    unsigned int id = glCreateShader(t);
    const char *src = s.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);
    int res;
    glGetShaderiv(id, GL_COMPILE_STATUS, &res);
    if (res == GL_FALSE)
    {
        int len;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> msg(len);
        glGetShaderInfoLog(id, len, &len, msg.data());
        std::cerr << "Shader compilation failed: " << msg.data() << std::endl;
        glDeleteShader(id);
        return 0;
    }
    return id;
}
unsigned int Shader::createShaderProgram(const std::string &vs, const std::string &fs)
{
    unsigned int p = glCreateProgram();
    unsigned int v = compileShader(GL_VERTEX_SHADER, vs);
    unsigned int f = compileShader(GL_FRAGMENT_SHADER, fs);
    if (v == 0 || f == 0)
        return 0;
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);
    glValidateProgram(p);
    glDeleteShader(v);
    glDeleteShader(f);
    return p;
}
