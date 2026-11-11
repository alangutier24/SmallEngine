#include "small_engine.h"

int main()
{
    SmallEngine app(1280, 720, "SmallEngine");

    if (!app.IsReady())
        return -1;

    app.Run();
    return 0;
}
