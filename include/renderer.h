#ifndef RENDERER_H
#define RENDERER_H

#include <glm/glm.hpp>

class Window;
class Mesh;
class Shader;
class Camera;

class Renderer
{
public:
    Renderer(Window& window);

    void Init();

    void BeginFrame(const Camera& camera);
    void EndFrame();

    void Clear();
    void SetClearColor(float r, float g, float b, float a = 1.0f);

    void Draw(const Mesh& mesh, Shader& shader, const glm::mat4& model);
    void DrawSubmesh(const Mesh& mesh, Shader& shader, const glm::mat4& model, size_t index);
    void DrawVolumetric(const Mesh& mesh, Shader& shader, const glm::mat4& modelm, float time);
    void DrawSmoke(const Mesh& mesh, Shader& shader, const glm::mat4& model, float time);

    void SetDepthTest(bool enabled);
    void SetDepthMask(bool enabled);
    void SetBlending(bool enabled);

    const glm::mat4& GetView()       const { return view; }
    const glm::mat4& GetProjection() const { return projection; }

private:
    void UpdateViewport();

private:
    Window& window;

    glm::mat4 view       = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    glm::vec3 cameraPos  = glm::vec3(0.0f);

    int lastWidth  = 0;
    int lastHeight = 0;

    float clearColor[4] = { 0.1f, 0.1f, 0.15f, 1.0f };
};

#endif