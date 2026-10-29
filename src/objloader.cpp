#include "objloader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <glm/gtc/type_ptr.hpp>
#include <cstdio>

static Material MaterialFromTinyObj(const tinyobj::material_t& m)
{
    Material mat;
    mat.name      = m.name;
    mat.ambient   = glm::make_vec3(m.ambient);
    mat.diffuse   = glm::make_vec3(m.diffuse);
    mat.specular  = glm::make_vec3(m.specular);
    mat.emission  = glm::make_vec3(m.emission);
    mat.shininess = m.shininess;
    mat.ior       = m.ior;
    mat.dissolve  = m.dissolve;
    mat.illum     = m.illum;
    return mat;
}

LoadedModel OBJLoader::Load(const std::string& objPath, const std::string& materialDir)
{
    LoadedModel result;

    tinyobj::ObjReaderConfig config;
    if (!materialDir.empty()) config.mtl_search_path = materialDir;
    config.triangulate = true;

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(objPath, config))
    {
        std::fprintf(stderr, "OBJLoader: fallo cargando %s\n%s\n",
                     objPath.c_str(), reader.Error().c_str());
        return result;
    }

    if (!reader.Warning().empty())
        std::fprintf(stderr, "OBJLoader: %s\n", reader.Warning().c_str());

    const tinyobj::attrib_t& attrib = reader.GetAttrib();
    const auto& shapes    = reader.GetShapes();
    const auto& tinyMats  = reader.GetMaterials();

    for (const auto& m : tinyMats)
        result.materials.push_back(MaterialFromTinyObj(m));

    std::vector<Vertex>  vertices;
    std::vector<Submesh> submeshes;

    // Agrupa por material: un submesh por material, no por shape.
    int materialCount = (int)tinyMats.size();
    int groupCount    = materialCount > 0 ? materialCount : 1;

    for (int targetMat = 0; targetMat < groupCount; ++targetMat)
    {
        unsigned int startVertex = (unsigned int)vertices.size();

        for (const auto& shape : shapes)
        {
            size_t indexOffset = 0;

            for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f)
            {
                int faceMat = shape.mesh.material_ids[f];
                size_t fv   = shape.mesh.num_face_vertices[f];

                bool matches = (materialCount > 0) ? (faceMat == targetMat) : true;
                if (!matches) { indexOffset += fv; continue; }

                for (size_t v = 0; v < fv; ++v)
                {
                    tinyobj::index_t idx = shape.mesh.indices[indexOffset + v];
                    Vertex vertex;

                    vertex.position = {
                        attrib.vertices[3 * idx.vertex_index + 0],
                        attrib.vertices[3 * idx.vertex_index + 1],
                        attrib.vertices[3 * idx.vertex_index + 2]
                    };

                    if (idx.normal_index >= 0)
                        vertex.normal = {
                            attrib.normals[3 * idx.normal_index + 0],
                            attrib.normals[3 * idx.normal_index + 1],
                            attrib.normals[3 * idx.normal_index + 2]
                        };

                    if (idx.texcoord_index >= 0)
                        vertex.texCoords = {
                            attrib.texcoords[2 * idx.texcoord_index + 0],
                            attrib.texcoords[2 * idx.texcoord_index + 1]
                        };

                    vertices.push_back(vertex);
                }

                indexOffset += fv;
            }
        }

        unsigned int added = (unsigned int)vertices.size() - startVertex;
        if (added > 0)
        {
            Submesh sub;
            sub.offset        = startVertex;
            sub.count         = added;
            sub.materialIndex = materialCount > 0 ? targetMat : -1;
            submeshes.push_back(sub);
        }
    }

    if (vertices.empty())
    {
        std::fprintf(stderr, "OBJLoader: %s no tiene vertices\n", objPath.c_str());
        return result;
    }

    result.mesh = std::make_unique<Mesh>(vertices);
    result.mesh->SetSubmeshes(submeshes);
    result.valid = true;

    return result;
}