#ifndef MESH_H
#define MESH_H

#include <glm/glm.hpp>
#include <vector>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

struct Submesh
{
    unsigned int offset        = 0;
    unsigned int count         = 0;
    int          materialIndex = -1;
};

class Mesh
{
public:
    Mesh(const std::vector<Vertex>& vertices);
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void Draw() const;
    void DrawSubmesh(size_t index) const;

    void SetSubmeshes(const std::vector<Submesh>& subs) { submeshes = subs; }
    const std::vector<Submesh>& GetSubmeshes() const { return submeshes; }
    size_t GetSubmeshCount() const { return submeshes.size(); }

    bool IsIndexed() const { return ebo != 0; }
    bool IsValid()   const { return vao != 0; }
    unsigned int GetElementCount() const { return elementCount; }

private:
    void Upload(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    void Release();

private:
    unsigned int vao = 0;
    unsigned int vbo = 0;
    unsigned int ebo = 0;

    unsigned int elementCount = 0;

    std::vector<Submesh> submeshes;
};

#endif