#include "renderer.h"
#include "window.h"
#include "mesh.h"
#include "shader.h"
#include "camera.h"
#include <glad/gl.h>

#include <array>
#include <numeric>
#include <algorithm>
#include <random>

#include <fstream>
#include <thread>


Renderer::Renderer(Window& w) : window(w) {}

void Renderer::UpdateViewport()
{
    int w = window.GetWidth();
    int h = window.GetHeight();

    if (w != lastWidth || h != lastHeight)
    {
        glViewport(0, 0, w, h);
        lastWidth  = w;
        lastHeight = h;
    }
}

void Renderer::BeginFrame(const Camera& camera)
{
    UpdateViewport();

    view       = camera.GetViewMatrix();
    projection = camera.GetProjectionMatrix(window.GetAspectRatio());
    cameraPos  = camera.GetPosition();

    Clear();
}

void Renderer::EndFrame()
{
    window.SwapBuffers();
}

void Renderer::Clear()
{
    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::SetClearColor(float r, float g, float b, float a)
{
    clearColor[0] = r; clearColor[1] = g;
    clearColor[2] = b; clearColor[3] = a;
}

void Renderer::Draw(const Mesh& mesh, Shader& shader, const glm::mat4& model)
{
    shader.Bind();
    shader.SetMat4("model",      model);
    shader.SetMat4("view",       view);
    shader.SetMat4("projection", projection);

    mesh.Draw();
}

void Renderer::DrawSubmesh(const Mesh& mesh, Shader& shader, const glm::mat4& model, size_t index)
{
    shader.Bind();
    shader.SetMat4("model",      model);
    shader.SetMat4("view",       view);
    shader.SetMat4("projection", projection);

    mesh.DrawSubmesh(index);
}

std::array<int, 512> makePermutation(unsigned seed = 2024) {
    std::array<int, 256> perm;
    std::iota(perm.begin(), perm.end(), 0);   // 0..255

    std::mt19937 rng(seed);
    std::shuffle(perm.begin(), perm.end(), rng);

    std::array<int, 512> p;
    for (int i = 0; i < 256; ++i)
        p[i] = p[i + 256] = perm[i];
    return p;
}

static bool intersectBox(const glm::vec3& orig, const glm::vec3& dir,
                         float& tNear, float& tFar) {
    // La division por 0 da +-inf igual que en GLSL, el min/max lo absorbe.
    glm::vec3 ta = (glm::vec3(-0.5f) - orig) / dir;
    glm::vec3 tb = (glm::vec3( 0.5f) - orig) / dir;
    glm::vec3 t1 = glm::min(ta, tb);
    glm::vec3 t2 = glm::max(ta, tb);
    tNear = std::max(t1.x, std::max(t1.y, t1.z));
    tFar  = std::min(t2.x, std::min(t2.y, t2.z));
    return tFar > 0.0f && tNear < tFar;
}

static const float PI = 3.14159265359f;

static float phase(float g, float cosTheta) {
    float denom = 1.0f + g * g - 2.0f * g * cosTheta;
    return (1.0f / (4.0f * PI)) * (1.0f - g * g) / (denom * std::sqrt(denom));
}
 
// Jitter: en el shader entra gl_FragCoord.xy; aqui entra (i+0.5, j+0.5).
static float ign(const glm::vec2& q) {
    return glm::fract(52.9829189f *
                      glm::fract(glm::dot(q, glm::vec2(0.06711056f, 0.00583715f))));
}

static inline float fade(float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}
 
static inline float grad(int hash, float x, float y, float z) {
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : (h == 12 || h == 14) ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

static inline float lerp(float a, float b, float t) { return a + t * (b - a); }
 
static float perlin(const glm::vec3& P) {

    auto p = makePermutation();

    int X = int(std::floor(P.x)) & 255;
    int Y = int(std::floor(P.y)) & 255;
    int Z = int(std::floor(P.z)) & 255;
 
    float x = P.x - std::floor(P.x);
    float y = P.y - std::floor(P.y);
    float z = P.z - std::floor(P.z);
 
    float u = fade(x);
    float v = fade(y);
    float w = fade(z);
 
    int A  = p[X]     + Y;
    int AA = p[A]     + Z;
    int AB = p[A + 1] + Z;
    int B  = p[X + 1] + Y;
    int BA = p[B]     + Z;
    int BB = p[B + 1] + Z;
 
    return lerp(
        lerp(lerp(grad(p[AA    ], x,        y,        z       ),
                  grad(p[BA    ], x - 1.0f, y,        z       ), u),
             lerp(grad(p[AB    ], x,        y - 1.0f, z       ),
                  grad(p[BB    ], x - 1.0f, y - 1.0f, z       ), u), v),
        lerp(lerp(grad(p[AA + 1], x,        y,        z - 1.0f),
                  grad(p[BA + 1], x - 1.0f, y,        z - 1.0f), u),
             lerp(grad(p[AB + 1], x,        y - 1.0f, z - 1.0f),
                  grad(p[BB + 1], x - 1.0f, y - 1.0f, z - 1.0f), u), v), w);
}

static const float NOISE_FREQ = 4.0f;
static const int   OCTAVES    = 4;
static const float LACUNARITY = 2.0f;
static const float H          = 0.4f;
static const float RADIUS     = 0.5f;
 
// uTime pasa de uniform a parametro.
static float density(const glm::vec3& pos, float uTime) {
    float dist    = std::min(1.0f, glm::length(pos) / RADIUS);
    float falloff = glm::smoothstep(0.8f, 1.0f, dist);
 
    float theta = uTime * 0.5f;
    glm::vec3 q = glm::vec3( std::cos(theta) * pos.x + std::sin(theta) * pos.z,
                   pos.y,
                  -std::sin(theta) * pos.x + std::cos(theta) * pos.z );
 
    q *= NOISE_FREQ;
    float fbm = 0.0f;
    for (int k = 0; k < OCTAVES; ++k) {
        fbm += perlin(q) * std::pow(LACUNARITY, -H * float(k));
        q   *= LACUNARITY;
    }
 
    return std::max(0.0f, fbm) * (1.0f - falloff);
}

 
static const int LIGHT_STEPS = 8;
 
static glm::vec3 integrate(const glm::vec3& rayOrigin, const glm::vec3& ray_dir,
                      const glm::vec2& fragCoord, float uTime) {
    const glm::vec3 background(0.2f, 0.3f, 0.3f);
 
    float tNear, tFar;
    if (!intersectBox(rayOrigin, ray_dir, tNear, tFar))
        return background;                 // <- el 'discard' del shader
    tNear = std::max(tNear, 0.0f);
 
    float sigma_a = 5.0f;
    float sigma_s = 5.0f;
    float sigma_t = sigma_a + sigma_s;
 
    float step_size = 0.03f;
    int   ns = std::max(1, int(std::ceil((tFar - tNear) / step_size)));
    step_size = (tFar - tNear) / float(ns);
 
    glm::vec3 light_dir   = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec3 light_color = glm::vec3(20.0f);
 
    float g  = 0.8f;
    float ph = phase(g, glm::dot(ray_dir, light_dir));
 
    float transmission = 1.0f;
    glm::vec3  result(0.0f);
 
    float jitter = ign(fragCoord);
 
    for (int n = 0; n < ns; ++n) {
        float t   = tNear + step_size * (float(n) + jitter);
        glm::vec3  pos = rayOrigin + ray_dir * t;
 
        float d = density(pos, uTime);
        if (d <= 0.0f) continue;
 
        float lNear, lFar;
        float lightAtten = 1.0f;
        if (intersectBox(pos, light_dir, lNear, lFar)) {
            float l_step = lFar / float(LIGHT_STEPS);
            float tau    = 0.0f;
            for (int m = 0; m < LIGHT_STEPS; ++m) {
                glm::vec3 lpos = pos + light_dir * l_step * (float(m) + 0.5f);
                tau += density(lpos, uTime) * l_step;
            }
            lightAtten = std::exp(-tau * sigma_t);
        }
 
        result += transmission * d * sigma_s * ph
                * lightAtten * light_color * step_size;
 
        transmission *= std::exp(-d * sigma_t * step_size);
    }
 
    return background * transmission + result;
}

void Renderer::DrawVolumetric(const Mesh& mesh, Shader& shader, const glm::mat4& model, float time)
{
    SetDepthMask(false);
    SetBlending(true);

    auto p = makePermutation();

    glm::vec3 localCam = glm::vec3(glm::inverse(model) * glm::vec4(cameraPos, 1.0f));

    shader.Bind();
    // tras crear/linkar el shader
    auto perm = makePermutation();                            // o como llames a glUseProgram
    shader.SetIntArray("p", perm.data(), int(perm.size()));
    shader.SetMat4("model",      model);
    shader.SetMat4("view",       view);
    shader.SetMat4("projection", projection);
    shader.SetVec3("vrayOrigin", localCam);

    mesh.Draw();

    SetBlending(false);
    SetDepthMask(true);
}

struct Grid {
    float *density;
    float dimension = 128;
    glm::vec3 bounds[2] = { {-30,-30,-30}, {30, 30, 30} };
    float resolution = 32;
};

void Renderer::DrawSmoke(const Mesh& mesh, Shader& shader, const glm::mat4& model, float time)
{

    auto p = makePermutation();

    glm::vec3 localCam = glm::vec3(glm::inverse(model) * glm::vec4(cameraPos, 1.0f));

    shader.Bind();
    // tras crear/linkar el shader
    auto perm = makePermutation();                            // o como llames a glUseProgram
    shader.SetMat4("model",      model);
    shader.SetMat4("view",       view);
    shader.SetMat4("projection", projection);
    shader.SetVec3("vrayOrigin", localCam);

    mesh.Draw();

    glUseProgram(shader.GetID());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, 0);
    glUniform1i(glGetUniformLocation(shader.GetID(), "uDensity"), 0);
}

void Renderer::SetDepthTest(bool enabled)
{
    enabled ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
}

void Renderer::SetDepthMask(bool enabled)
{
    glDepthMask(enabled ? GL_TRUE : GL_FALSE);
}

void Renderer::SetBlending(bool enabled)
{
    enabled ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
}

void Renderer::Init()
{
    glEnable(GL_DEPTH_TEST);
    int depthBits = 0;
    glGetIntegerv(GL_DEPTH_BITS, &depthBits);
    std::printf("depth bits: %d\n", depthBits);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    UpdateViewport();


    auto p = makePermutation();
 
    const int   width  = 640;
    const int   height = 480;
    const float fov    = glm::radians(45.0f);
    const float uTime  = 0.0f;
 
    const float aspect = float(width) / float(height);
    const float scale  = std::tan(fov * 0.5f);
 
    // Camara: lookAt da world->camera, la invertimos para camera->world.
    glm::mat4 cameraToWorld = glm::inverse(
        glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f),   // eye
                    glm::vec3(0.0f),               // target
                    glm::vec3(0.0f, 1.0f, 0.0f))); // up
 
    const glm::vec3 origin = glm::vec3(cameraToWorld[3]);
 
    std::vector<glm::vec3> framebuffer(size_t(width) * height);
 
    // El doble bucle que en GPU hace el rasterizador. Aqui lo repartimos
    // en bandas de filas entre hilos, que es el unico paralelismo que hay.
    auto renderBand = [&](int y0, int y1) {
        for (int j = y0; j < y1; ++j) {
            for (int i = 0; i < width; ++i) {
                float px = (2.0f * (i + 0.5f) / float(width)  - 1.0f) * aspect * scale;
                float py = (1.0f - 2.0f * (j + 0.5f) / float(height)) * scale;
 
                glm::vec3 dir = glm::normalize(
                    glm::vec3(cameraToWorld * glm::vec4(px, py, -1.0f, 0.0f)));
 
                framebuffer[size_t(j) * width + i] =
                    integrate(origin, dir, glm::vec2(i + 0.5f, j + 0.5f), uTime);
            }
        }
    };
 
    unsigned nThreads = std::max(1u, std::thread::hardware_concurrency());
    std::vector<std::thread> pool;
    int rowsPerThread = (height + int(nThreads) - 1) / int(nThreads);
    for (unsigned t = 0; t < nThreads; ++t) {
        int y0 = int(t) * rowsPerThread;
        int y1 = std::min(y0 + rowsPerThread, height);
        if (y0 < y1) pool.emplace_back(renderBand, y0, y1);
    }
    for (auto& th : pool) th.join();
 
    // PPM binario. Sin gamma, igual que el shader (no hay sRGB writeback).
    std::ofstream ofs("out.ppm", std::ios::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (const glm::vec3& c : framebuffer) {
        for (int k = 0; k < 3; ++k) {
            float v = glm::clamp(c[k], 0.0f, 1.0f);
            unsigned char b = static_cast<unsigned char>(v * 255.0f + 0.5f);
            ofs.write(reinterpret_cast<const char*>(&b), 1);
        }
    }
    ofs.close();


    int res = 128;
    std::vector<float> data(size_t(res) * res * res);

    // opcion A: leer el cache
    // cacheFile.read((char*)data.data(), sizeof(float) * data.size());
    // opcion B: bakear con tu density() de CPU
    for (int z = 0; z < res; ++z)
    for (int y = 0; y < res; ++y)
    for (int x = 0; x < res; ++x) {
        glm::vec3 pos = (glm::vec3(x, y, z) + 0.5f) / float(res) - 0.5f;  // -> [-0.5, 0.5]
        data[(size_t(z) * res + y) * res + x] = density(pos, 0.0f);
    }

    GLuint densityTex;
    glGenTextures(1, &densityTex);
    glBindTexture(GL_TEXTURE_3D, densityTex);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, res, res, res, 0,
                GL_RED, GL_FLOAT, data.data());

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}