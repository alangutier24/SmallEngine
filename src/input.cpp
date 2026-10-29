#include "input.h"

Input::Input(Window& w) : window(w) {}

void Input::Update()
{
    for (int i = FIRST_KEY; i < KEY_COUNT; ++i)
    {
        previousKeys[i] = currentKeys[i];
        currentKeys[i]  = window.IsKeyPressed(i);
    }

    double x, y;
    window.GetCursorPos(x, y);
    mousePos = glm::vec2((float)x, (float)y);

    if (firstMouse)
    {
        lastMouse  = mousePos;
        firstMouse = false;
    }

    mouseDelta.x = mousePos.x - lastMouse.x;
    mouseDelta.y = lastMouse.y - mousePos.y;
    lastMouse    = mousePos;
}

bool Input::IsKeyDown(int key) const
{
    if (key < FIRST_KEY || key >= KEY_COUNT) return false;
    return currentKeys[key];
}

bool Input::WasKeyPressed(int key) const
{
    if (key < FIRST_KEY || key >= KEY_COUNT) return false;
    return currentKeys[key] && !previousKeys[key];
}

bool Input::WasKeyReleased(int key) const
{
    if (key < FIRST_KEY || key >= KEY_COUNT) return false;
    return !currentKeys[key] && previousKeys[key];
}

void Input::SetCursorLocked(bool locked)
{
    cursorLocked = locked;
    glfwSetInputMode(window.GetHandle(), GLFW_CURSOR,
                     locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}