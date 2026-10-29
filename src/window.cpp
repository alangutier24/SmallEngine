#include "window.h"

#include <glad/gl.h>
#include "window.h"

#include <cstdio>

Window::Window(int w, int h, const char* title)
{
    if (!glfwInit())
    {
        std::fprintf(stderr, "Error: glfwInit fallo\n");
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(w, h, title, nullptr, nullptr);
    if (!window)
    {
        std::fprintf(stderr, "Error: glfwCreateWindow fallo\n");
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL(glfwGetProcAddress))
    {
        std::fprintf(stderr, "Error: gladLoadGL fallo\n");
        return;
    }

    glfwSwapInterval(1);

    glfwGetFramebufferSize(window, &width, &height);

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
}

Window::~Window()
{
    if (window) glfwDestroyWindow(window);
    glfwTerminate();
}

bool Window::ShouldClose() const
{
    return glfwWindowShouldClose(window);
}

void Window::SwapBuffers()
{
    glfwSwapBuffers(window);
}

void Window::PollEvents()
{
    glfwPollEvents();
}

bool Window::IsKeyPressed(int key) const
{
    return glfwGetKey(window, key) == GLFW_PRESS;
}

void Window::GetCursorPos(double& x, double& y) const
{
    glfwGetCursorPos(window, &x, &y);
}

void Window::FramebufferSizeCallback(GLFWwindow* handle, int w, int h)
{
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(handle));
    if (!self) return;

    self->width  = w;
    self->height = h;
}