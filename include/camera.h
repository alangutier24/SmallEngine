#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
    Camera(glm::vec3 startPos = glm::vec3(0.0f, 0.0f, 3.0f));

    void Rotate(float yawOffset, float pitchOffset);
    void Move(const glm::vec3& localDir, float deltaTime);

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix(float aspect) const;

    const glm::vec3& GetPosition() const { return position; }
    const glm::vec3& GetFront()    const { return front; }

public:
    float speed       = 3.5f;
    float sensitivity = 0.1f;
    float fov         = 45.0f;
    float nearPlane   = 0.1f;
    float farPlane    = 100.0f;

private:
    void UpdateVectors();

private:
    glm::vec3 position;
    glm::vec3 front   = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 right   = glm::vec3(1.0f, 0.0f,  0.0f);
    glm::vec3 up      = glm::vec3(0.0f, 1.0f,  0.0f);
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f,  0.0f);

    float yaw   = -90.0f;
    float pitch =   0.0f;
};

#endif