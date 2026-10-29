#include "application.h"

int main()
{
    Application app(1280, 720, "SmallEngine");

    if (!app.IsReady())
        return -1;

    app.Run();
    return 0;
}