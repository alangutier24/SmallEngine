#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>
#include <string>

class Shader;

struct Material
{
    std::string name;

    glm::vec3 ambient   = glm::vec3(0.1f);
    glm::vec3 diffuse   = glm::vec3(0.8f);
    glm::vec3 specular  = glm::vec3(0.5f);
    glm::vec3 emission  = glm::vec3(0.0f);

    float shininess = 32.0f;
    float ior       = 1.0f;
    float dissolve  = 1.0f;
    int   illum     = 2;

    unsigned int diffuseMap = 0;
    unsigned int ambientMap = 0;

    void Apply(Shader& shader) const;
};

#endif