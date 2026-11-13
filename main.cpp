#include "small_engine.h"

#include <cstdio>
#include <cstring>

int main(int argc, char** argv)
{
    // --cpu-reference [archivo]: renderiza el volumen con el integrador de CPU,
    // escribe un PPM y sale. No crea ventana ni contexto GL.
    if (argc > 1 && std::strcmp(argv[1], "--cpu-reference") == 0)
    {
        const char* path = argc > 2 ? argv[2] : "out.ppm";
        if (!Renderer::RenderCpuReference(path))
        {
            std::fprintf(stderr, "SmallEngine: no se pudo escribir %s\n", path);
            return 1;
        }
        std::printf("Referencia de CPU escrita en %s\n", path);
        return 0;
    }

    SmallEngine app(1280, 720, "SmallEngine");

    if (!app.IsReady())
        return -1;

    app.Run();
    return 0;
}
