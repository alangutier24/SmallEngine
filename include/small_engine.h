#ifndef SMALL_ENGINE_H
#define SMALL_ENGINE_H

#include "window.h"
#include "input.h"
#include "renderer.h"
#include "camera.h"
#include "shader.h"
#include "mesh.h"
#include "material.h"

#include <memory>
#include <vector>

class SmallEngine
{
public:
    SmallEngine(int width, int height, const char* title);
    ~SmallEngine();

    SmallEngine(const SmallEngine&) = delete;
    SmallEngine& operator=(const SmallEngine&) = delete;

    void Run();

    bool IsReady() const { return resourcesReady; }

private:
    bool LoadResources();
    void ProcessInput(float deltaTime);
    void Update(float deltaTime);
    void Render();

private:
    Window   window;
    Input    input;
    Renderer renderer;
    Camera   camera;

    std::unique_ptr<Shader> meshShader;
    std::unique_ptr<Shader> volumeShader;

    std::unique_ptr<Mesh> whale;
    std::unique_ptr<Mesh> fogCube;

    std::vector<Material> materials;

    glm::mat4 whaleTransform   = glm::mat4(1.0f);
    glm::mat4 fogTransform     = glm::mat4(1.0f);

    float lastFrameTime = 0.0f;
    bool  resourcesReady = false;
};

#endif