#include "application.h"

#include "objloader.h"
#include <memory>

#include <cstdio>

Application::Application(int width, int height, const char* title)
    : window(width, height, title),
      input(window),
      renderer(window),
      camera(glm::vec3(0.0f, 0.0f, 5.0f))
{
    renderer.Init();
    renderer.SetClearColor(0.05f, 0.06f, 0.09f);

    input.SetCursorLocked(true);

    resourcesReady = LoadResources();
}

Application::~Application() = default;

bool Application::LoadResources()
{
    meshShader   = std::make_unique<Shader>(RESOURCE_DIR "/shaders/mtl.vert",
                                            RESOURCE_DIR "/shaders/mtl.frag");
    volumeShader = std::make_unique<Shader>(RESOURCE_DIR "/shaders/smoke.vert",
                                            RESOURCE_DIR "/shaders/smoke.frag");

    if (!meshShader->IsValid() || !volumeShader->IsValid())
    {
        std::fprintf(stderr, "Application: shaders invalidos\n");
        return false;
    }

    // TODO: cargar la ballena con tinyobj -> vertices, submeshes, materials
    // whale = std::make_unique<Mesh>(vertices);
    // whale->SetSubmeshes(submeshes);

    std::vector<Vertex> cubeVerts(8);
    cubeVerts[0].position = { -0.5f, -0.5f, -0.5f };
    cubeVerts[1].position = {  0.5f, -0.5f, -0.5f };
    cubeVerts[2].position = {  0.5f,  0.5f, -0.5f };
    cubeVerts[3].position = { -0.5f,  0.5f, -0.5f };
    cubeVerts[4].position = { -0.5f, -0.5f,  0.5f };
    cubeVerts[5].position = {  0.5f, -0.5f,  0.5f };
    cubeVerts[6].position = {  0.5f,  0.5f,  0.5f };
    cubeVerts[7].position = { -0.5f,  0.5f,  0.5f };

    std::vector<unsigned int> cubeIndices = {
        0,1,2, 2,3,0,   4,5,6, 6,7,4,
        7,3,0, 0,4,7,   6,2,1, 1,5,6,
        0,1,5, 5,4,0,   3,2,6, 6,7,3
    };

    fogCube = std::make_unique<Mesh>(cubeVerts, cubeIndices);

    fogTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.5f));
    fogTransform = glm::scale(fogTransform, glm::vec3(12.0f));

    whaleTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f));
    whaleTransform = glm::scale(whaleTransform, glm::vec3(1.0f));

    LoadedModel model = OBJLoader::Load(RESOURCE_DIR "/obj/OBJ/whale.obj", RESOURCE_DIR "/obj/OBJ");
    if (!model.valid) return false;

    whale     = std::move(model.mesh);
    materials = std::move(model.materials);

    std::printf("--- recursos ---\n");
    std::printf("meshShader:   %s (id=%u)\n",
                meshShader->IsValid() ? "ok" : "FALLO", meshShader->GetID());
    std::printf("volumeShader: %s (id=%u)\n",
                volumeShader->IsValid() ? "ok" : "FALLO", volumeShader->GetID());
    std::printf("whale:   %s, submeshes=%zu, materials=%zu\n",
                whale && whale->IsValid() ? "ok" : "FALLO",
                whale ? whale->GetSubmeshCount() : 0,
                materials.size());
    std::printf("fogCube: %s, indexado=%d, elementos=%u\n",
                fogCube && fogCube->IsValid() ? "ok" : "FALLO",
                fogCube ? fogCube->IsIndexed() : 0,
                fogCube ? fogCube->GetElementCount() : 0);
    std::printf("----------------\n");

    return true;
}

void Application::Run()
{
    if (!resourcesReady) return;

    lastFrameTime = (float)glfwGetTime();

    while (!window.ShouldClose())
    {
        float now       = (float)glfwGetTime();
        float deltaTime = now - lastFrameTime;
        lastFrameTime   = now;

        window.PollEvents();
        input.Update();

        ProcessInput(deltaTime);
        Update(deltaTime);
        Render();
    }
}

void Application::ProcessInput(float deltaTime)
{
    if (input.WasKeyPressed(GLFW_KEY_ESCAPE))
        input.SetCursorLocked(!input.IsCursorLocked());

    if (input.IsCursorLocked())
    {
        glm::vec2 delta = input.GetMouseDelta();
        camera.Rotate(delta.x, delta.y);
    }

    glm::vec3 dir(0.0f);
    if (input.IsKeyDown(GLFW_KEY_W)) dir.z += 1.0f;
    if (input.IsKeyDown(GLFW_KEY_S)) dir.z -= 1.0f;
    if (input.IsKeyDown(GLFW_KEY_D)) dir.x += 1.0f;
    if (input.IsKeyDown(GLFW_KEY_A)) dir.x -= 1.0f;
    if (input.IsKeyDown(GLFW_KEY_E)) dir.y += 1.0f;
    if (input.IsKeyDown(GLFW_KEY_Q)) dir.y -= 1.0f;

    if (glm::length(dir) > 0.0f)
        camera.Move(glm::normalize(dir), deltaTime);
}

void Application::Update(float deltaTime)
{
    (void)deltaTime;
}

void Application::Render()
{
    renderer.BeginFrame(camera);

    if (whale)
    {
        meshShader->Bind();
        for (size_t i = 0; i < whale->GetSubmeshCount(); ++i)
        {
            int matIndex = whale->GetSubmeshes()[i].materialIndex;
            if (matIndex >= 0 && matIndex < (int)materials.size())
                materials[matIndex].Apply(*meshShader);

            renderer.DrawSubmesh(*whale, *meshShader, whaleTransform, i);
        }
    }

    if (fogCube)
        renderer.DrawVolumetric(*fogCube, *volumeShader, fogTransform, (float)glfwGetTime());

    renderer.EndFrame();
}