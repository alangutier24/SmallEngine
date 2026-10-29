#ifndef INPUT_H
#define INPUT_H

#include <glm/glm.hpp>
#include "window.h"

class Input
{
public:
    Input(Window& window);

    void Update();

    bool IsKeyDown(int key) const;
    bool WasKeyPressed(int key) const;
    bool WasKeyReleased(int key) const;

    glm::vec2 GetMouseDelta() const { return mouseDelta; }
    glm::vec2 GetMousePos()   const { return mousePos; }

    void SetCursorLocked(bool locked);
    bool IsCursorLocked() const { return cursorLocked; }

private:
    Window& window;

    static constexpr int FIRST_KEY = GLFW_KEY_SPACE;
    static constexpr int KEY_COUNT = GLFW_KEY_LAST + 1;

    bool currentKeys[KEY_COUNT]  = { false };
    bool previousKeys[KEY_COUNT] = { false };

    glm::vec2 mousePos   = glm::vec2(0.0f);
    glm::vec2 lastMouse  = glm::vec2(0.0f);
    glm::vec2 mouseDelta = glm::vec2(0.0f);

    bool firstMouse   = true;
    bool cursorLocked = false;
};

#endif