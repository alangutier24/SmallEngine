#include "material.h"
#include "shader.h"

#include <OpenGL/gl3.h>

void Material::Apply(Shader& shader) const
{
    shader.SetVec3 ("uKa", ambient);
    shader.SetVec3 ("uKd", diffuse);
    //shader.SetVec3 ("uKs", specular);
    shader.SetVec3 ("uKe", emission);
    //shader.SetFloat("uNs", shininess);
    //shader.SetFloat("uNi", ior);
    shader.SetFloat("ud",  dissolve);
    shader.SetInt  ("uIllum", illum);

    /*
    shader.SetInt("uHasMapKd", diffuseMap != 0);
    shader.SetInt("uHasMapKa", ambientMap != 0);

    if (diffuseMap)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        shader.SetInt("uMapKd", 0);
    }

    if (ambientMap)
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, ambientMap);
        shader.SetInt("uMapKa", 1);
    }
    */
}