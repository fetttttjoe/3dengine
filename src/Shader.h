// Renderer/Shader.h
#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

class Shader {
 public:
  Shader(const std::string& vertexPath, const std::string& fragmentPath);
  Shader(const char* vertexSource, const char* fragmentSource, bool fromMemory);

  ~Shader();

  void Bind() const;
  void Unbind() const;

  // core uniform setters
  void SetUniform1ui(const std::string& name, uint32_t value);
  void SetUniform1i(const std::string& name, int value);
  void SetUniform1f(const std::string& name, float value);

  void SetUniform3f(const std::string& name, float v0, float v1, float v2);
  void SetUniform4f(const std::string& name, float v0, float v1, float v2,
                    float v3);
  void SetUniform4f(const std::string &name, const glm::vec4& value);
  void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);

  // convenience overloads
  void SetUniformVec3(const std::string& name, const glm::vec3& v);
  void SetUniformVec4(const std::string& name, const glm::vec4& v);

  unsigned int GetRendererID() const { return m_RendererID; }

 private:
  std::string loadShaderSource(const std::string& filepath);
  unsigned int compileShader(unsigned int type, const std::string& source);
  unsigned int createShaderProgram(const std::string& vs,
                                   const std::string& fs);
  int getUniformLocation(const std::string& name);

  unsigned int m_RendererID = 0;
  std::unordered_map<std::string, int> m_UniformLocationCache;
};