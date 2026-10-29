#include "mesh.h"
#include <glad/gl.h>
#include <cstddef>

Mesh::Mesh(const std::vector<Vertex>& vertices)
{
    Upload(vertices, {});
}

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
{
    Upload(vertices, indices);
}

void Mesh::Upload(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
{
    if (vertices.empty()) return;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(Vertex),
                 vertices.data(),
                 GL_STATIC_DRAW);

    if (!indices.empty())
    {
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     indices.size() * sizeof(unsigned int),
                     indices.data(),
                     GL_STATIC_DRAW);

        elementCount = (unsigned int)indices.size();
    }
    else
    {
        elementCount = (unsigned int)vertices.size();
    }

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, texCoords));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::Release()
{
    if (ebo) glDeleteBuffers(1, &ebo);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);

    vao = vbo = ebo = 0;
    elementCount = 0;
}

Mesh::~Mesh()
{
    Release();
}

Mesh::Mesh(Mesh&& other) noexcept
    : vao(other.vao), vbo(other.vbo), ebo(other.ebo),
      elementCount(other.elementCount),
      submeshes(std::move(other.submeshes))
{
    other.vao = other.vbo = other.ebo = 0;
    other.elementCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    if (this != &other)
    {
        Release();

        vao          = other.vao;
        vbo          = other.vbo;
        ebo          = other.ebo;
        elementCount = other.elementCount;
        submeshes    = std::move(other.submeshes);

        other.vao = other.vbo = other.ebo = 0;
        other.elementCount = 0;
    }
    return *this;
}

void Mesh::Draw() const
{
    if (!vao) return;

    glBindVertexArray(vao);

    if (ebo)
        glDrawElements(GL_TRIANGLES, elementCount, GL_UNSIGNED_INT, nullptr);
    else
        glDrawArrays(GL_TRIANGLES, 0, elementCount);

    glBindVertexArray(0);
}

void Mesh::DrawSubmesh(size_t index) const
{
    if (!vao || index >= submeshes.size()) return;

    const Submesh& sub = submeshes[index];
    if (sub.count == 0) return;

    glBindVertexArray(vao);

    if (ebo)
        glDrawElements(GL_TRIANGLES, sub.count, GL_UNSIGNED_INT,
                       (void*)(sub.offset * sizeof(unsigned int)));
    else
        glDrawArrays(GL_TRIANGLES, sub.offset, sub.count);

    glBindVertexArray(0);
}
