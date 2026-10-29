#include "shader.h"

#include <OpenGL/gl3.h>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <glm/gtc/type_ptr.hpp>

std::string Shader::ReadFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::fprintf(stderr, "Shader: no se pudo abrir %s\n", path.c_str());
        return "";
    }

    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

unsigned int Shader::CompileStage(unsigned int type, const std::string& source,
                                  const std::string& debugName)
{
    if (source.empty()) return 0;

    unsigned int stage = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(stage, 1, &src, nullptr);
    glCompileShader(stage);

    int success = 0;
    glGetShaderiv(stage, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[1024];
        glGetShaderInfoLog(stage, sizeof(log), nullptr, log);
        std::fprintf(stderr, "Shader: fallo compilando %s\n%s\n", debugName.c_str(), log);
        glDeleteShader(stage);
        return 0;
    }

    return stage;
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
{
    std::string vertexSource   = ReadFile(vertexPath);
    std::string fragmentSource = ReadFile(fragmentPath);

    unsigned int vertex   = CompileStage(GL_VERTEX_SHADER,   vertexSource,   vertexPath);
    unsigned int fragment = CompileStage(GL_FRAGMENT_SHADER, fragmentSource, fragmentPath);

    if (vertex == 0 || fragment == 0)
    {
        if (vertex)   glDeleteShader(vertex);
        if (fragment) glDeleteShader(fragment);
        return;
    }

    id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);

    int success = 0;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success)
    {
        char log[1024];
        glGetProgramInfoLog(id, sizeof(log), nullptr, log);
        std::fprintf(stderr, "Shader: fallo enlazando %s + %s\n%s\n",
                     vertexPath.c_str(), fragmentPath.c_str(), log);
        glDeleteProgram(id);
        id = 0;
    }

    glDetachShader(id ? id : 0, vertex);
    glDetachShader(id ? id : 0, fragment);
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader()
{
    if (id) glDeleteProgram(id);
}

Shader::Shader(Shader&& other) noexcept
    : id(other.id), uniformCache(std::move(other.uniformCache))
{
    other.id = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept
{
    if (this != &other)
    {
        if (id) glDeleteProgram(id);

        id           = other.id;
        uniformCache = std::move(other.uniformCache);
        other.id     = 0;
    }
    return *this;
}

void Shader::Bind() const
{
    glUseProgram(id);
}

void Shader::Unbind() const
{
    glUseProgram(0);
}

int Shader::GetUniformLocation(const std::string& name)
{
    auto it = uniformCache.find(name);
    if (it != uniformCache.end()) return it->second;

    int location = glGetUniformLocation(id, name.c_str());
    if (location == -1)
        std::fprintf(stderr, "Shader: uniform '%s' no encontrado\n", name.c_str());

    uniformCache[name] = location;
    return location;
}

void Shader::SetBool(const std::string& name, bool value)
{
    glUniform1i(GetUniformLocation(name), (int)value);
}

void Shader::SetInt(const std::string& name, int value)
{
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetFloat(const std::string& name, float value)
{
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetVec2(const std::string& name, const glm::vec2& value)
{
    glUniform2fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value)
{
    glUniform3fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetVec4(const std::string& name, const glm::vec4& value)
{
    glUniform4fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetMat3(const std::string& name, const glm::mat3& value)
{
    glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetMat4(const std::string& name, const glm::mat4& value)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetIntArray(const std::string& name, const int* values, int count)
{
    glUniform1iv(GetUniformLocation(name), count, values);
}
