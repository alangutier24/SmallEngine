#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

class Window
{
public:
    Window(int width, int height, const char* title);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool ShouldClose() const;
    void SwapBuffers();
    void PollEvents();

    bool IsKeyPressed(int key) const;
    void GetCursorPos(double& x, double& y) const;

    int  GetWidth()  const { return width; }
    int  GetHeight() const { return height; }
    float GetAspectRatio() const { return (float)width / (float)height; }

    GLFWwindow* GetHandle() const { return window; }

private:
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

private:
    GLFWwindow* window = nullptr;
    int width  = 0;
    int height = 0;
};

#endif