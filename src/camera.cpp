#include "camera.h"

Camera::Camera(glm::vec3 startPos) : position(startPos)
{
    UpdateVectors();
}

void Camera::UpdateVectors()
{
    glm::vec3 dir;
    dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    dir.y = sin(glm::radians(pitch));
    dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    front = glm::normalize(dir);
    right = glm::normalize(glm::cross(front, worldUp));
    up    = glm::normalize(glm::cross(right, front));
}

void Camera::Rotate(float yawOffset, float pitchOffset)
{
    yaw   += yawOffset   * sensitivity;
    pitch += pitchOffset * sensitivity;

    if (pitch >  89.0f) pitch =  89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    UpdateVectors();
}

void Camera::Move(const glm::vec3& localDir, float deltaTime)
{
    position += front   * localDir.z * speed * deltaTime;
    position += right   * localDir.x * speed * deltaTime;
    position += worldUp * localDir.y * speed * deltaTime;
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::GetProjectionMatrix(float aspect) const
{
    return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
}