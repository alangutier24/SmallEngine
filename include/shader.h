#ifndef SHADER_H
#define SHADER_H

#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

class Shader
{
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    void Bind() const;
    void Unbind() const;

    bool IsValid() const { return id != 0; }
    unsigned int GetID() const { return id; }

    void SetBool (const std::string& name, bool  value);
    void SetInt  (const std::string& name, int   value);
    void SetFloat(const std::string& name, float value);
    void SetVec2 (const std::string& name, const glm::vec2& value);
    void SetVec3 (const std::string& name, const glm::vec3& value);
    void SetVec4 (const std::string& name, const glm::vec4& value);
    void SetMat3 (const std::string& name, const glm::mat3& value);
    void SetMat4 (const std::string& name, const glm::mat4& value);
    void SetIntArray(const std::string& name, const int* values, int count);

private:
    static std::string ReadFile(const std::string& path);
    static unsigned int CompileStage(unsigned int type, const std::string& source,
                                     const std::string& debugName);

    int GetUniformLocation(const std::string& name);

private:
    unsigned int id = 0;
    std::unordered_map<std::string, int> uniformCache;
};

#endif