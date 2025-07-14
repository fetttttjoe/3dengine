#include "Shader.h"

#include <glad/glad.h>

#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "Core/Log.h"

Shader::Shader(const std::string& vp, const std::string& fp) : m_RendererID(0) {
  std::string vsSrc;
  std::string fsSrc;
  try {
    vsSrc = loadShaderSource(vp);
    fsSrc = loadShaderSource(fp);
  } catch (const std::exception& e) {
    Log::Debug("!!! SHADER FILE ERROR: ", e.what());
    throw;
  }

  m_RendererID = createShaderProgram(vsSrc, fsSrc);
}

Shader::Shader(const char* vertexSource, const char* fragmentSource,
               bool fromMemory)
    : m_RendererID(0) {
  if (vertexSource && fragmentSource) {
    m_RendererID = createShaderProgram(vertexSource, fragmentSource);
  }
}

Shader::~Shader() {
  Log::Debug("DELETING shader program ID: ", m_RendererID);
  if (m_RendererID) glDeleteProgram(m_RendererID);
}

void Shader::Bind() const {
  if (m_RendererID) glUseProgram(m_RendererID);
}

void Shader::Unbind() const { glUseProgram(0); }

void Shader::SetUniform1ui(const std::string& name, uint32_t value) {
  glUniform1ui(getUniformLocation(name), value);
}

void Shader::SetUniform1i(const std::string& name, int value) {
  glUniform1i(getUniformLocation(name), value);
}

void Shader::SetUniform1f(const std::string& name, float value) {
  glUniform1f(getUniformLocation(name), value);
}

void Shader::SetUniform3f(const std::string& name, float v0, float v1,
                          float v2) {
  glUniform3f(getUniformLocation(name), v0, v1, v2);
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2,
                          float v3) {
  glUniform4f(getUniformLocation(name), v0, v1, v2, v3);
}
void Shader::SetUniform4f(const std::string& name, const glm::vec4& value) {
  glUniform4f(getUniformLocation(name), value.r, value.g, value.b, value.a);
}
void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& m) {
  glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(m));
}

void Shader::SetUniformVec3(const std::string& name, const glm::vec3& v) {
  SetUniform3f(name, v.x, v.y, v.z);
}

void Shader::SetUniformVec4(const std::string& name, const glm::vec4& v) {
  SetUniform4f(name, v.x, v.y, v.z, v.w);
}
int Shader::getUniformLocation(const std::string& name) {
  if (m_UniformLocationCache.count(name)) {
    return m_UniformLocationCache[name];
  }

  if (m_RendererID == 0) {
    Log::Debug(
        "    [getUniformLocation] FATAL: Cannot get uniform location because "
        "shader program ID is 0.");
    return -1;
  }

  int loc = glGetUniformLocation(m_RendererID, name.c_str());
  m_UniformLocationCache[name] = loc;

  return loc;
}

std::string Shader::loadShaderSource(const std::string& filepath) {
  std::ifstream in(filepath);
  if (!in.is_open()) {
    throw std::runtime_error("Could not open shader file: " + filepath);
  }
  std::stringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

unsigned int Shader::compileShader(unsigned int type,
                                   const std::string& source) {
  unsigned int id = glCreateShader(type);
  const char* src = source.c_str();
  glShaderSource(id, 1, &src, nullptr);
  glCompileShader(id);

  int status;
  glGetShaderiv(id, GL_COMPILE_STATUS, &status);
  if (!status) {
    int len;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
    std::vector<char> msg(len);
    glGetShaderInfoLog(id, len, &len, msg.data());
    const char* shaderTypeStr =
        (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
    std::string errorMsg = "SHADER COMPILE ERROR (";
    errorMsg += shaderTypeStr;
    errorMsg += "): ";
    errorMsg += msg.data();
    glDeleteShader(id);
    throw std::runtime_error(errorMsg);
  }
  return id;
}

unsigned int Shader::createShaderProgram(const std::string& vs,
                                         const std::string& fs) {
  unsigned int prog = glCreateProgram();
  unsigned int v = 0;
  unsigned int f = 0;
  try {
    v = compileShader(GL_VERTEX_SHADER, vs);
    f = compileShader(GL_FRAGMENT_SHADER, fs);
  } catch (...) {
    glDeleteShader(v);
    glDeleteShader(f);
    glDeleteProgram(prog);
    throw;
  }

  glAttachShader(prog, v);
  glAttachShader(prog, f);
  glLinkProgram(prog);

  int status;
  glGetProgramiv(prog, GL_LINK_STATUS, &status);
  if (!status) {
    int len;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
    std::vector<char> msg(len);
    glGetProgramInfoLog(prog, len, &len, msg.data());
    std::string errorMsg = "SHADER LINK ERROR: ";
    errorMsg += msg.data();
    glDeleteProgram(prog);
    glDeleteShader(v);
    glDeleteShader(f);
    throw std::runtime_error(errorMsg);
  }

  glValidateProgram(prog);
  glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
  if (!status) {
    int len;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
    std::vector<char> msg(len);
    glGetProgramInfoLog(prog, len, &len, msg.data());
    std::string errorMsg = "SHADER VALIDATION ERROR: ";
    errorMsg += msg.data();
    glDeleteProgram(prog);
    glDeleteShader(v);
    glDeleteShader(f);
    throw std::runtime_error(errorMsg);
  }

  glDeleteShader(v);
  glDeleteShader(f);
  return prog;
}