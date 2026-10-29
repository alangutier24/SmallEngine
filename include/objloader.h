#ifndef OBJLOADER_H
#define OBJLOADER_H

#include "mesh.h"
#include "material.h"

#include <memory>
#include <string>
#include <vector>

struct LoadedModel
{
    std::unique_ptr<Mesh>  mesh;
    std::vector<Material>  materials;
    bool valid = false;
};

class OBJLoader
{
public:
    static LoadedModel Load(const std::string& objPath, const std::string& materialDir = "");
};

#endif