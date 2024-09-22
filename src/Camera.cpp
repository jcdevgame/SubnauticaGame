#define GLM_ENABLE_EXPERIMENTAL
#include "Camera.hpp"
#include <glm/gtx/rotate_vector.hpp>
#include <glad/glad.h>

Camera::Camera()
{
    // Assuming we are at the origin
    mEye = glm::vec3(0.0f, 7.0f, 0.0f);

    // Assuming we are looking out into the world
    // This is along -z because otherwise we'd be looking behind us.
    mViewDirection = glm::vec3(0.0f, 0.0f, -1.0f);
    // Assuming we start on a perfect plane
    mUpVector = glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(mEye, mEye + mViewDirection, mUpVector);
}

void Camera::MouseLook(int mouseX, int mouseY)
{
    if (enabled == true)
    {
        glm::vec2 currentMouse = glm::vec2(mouseX, mouseY);

        static bool firstLook = true;
        if (firstLook) {
            mOldMousePosition = currentMouse;
            firstLook = false;
        }

        glm::vec2 mouseDelta = mOldMousePosition - currentMouse;

        // Horizontal rotation (around the up vector)
        mViewDirection = glm::rotate(mViewDirection, glm::radians(mouseDelta.x), mUpVector);

        // Vertical rotation (around the right vector)
        glm::vec3 rightVector = glm::cross(mViewDirection, mUpVector);
        mViewDirection = glm::rotate(mViewDirection, glm::radians(mouseDelta.y), rightVector);

        mOldMousePosition = currentMouse;
    }
}

void Camera::MoveForward(float speed)
{
    //Simple -- but not yet correct L
    //plz fix

    mEye += (mViewDirection * speed);
}

void Camera::MoveBackward(float speed)
{
    mEye -= (mViewDirection * speed);
}

void Camera::MoveLeft(float speed)
{
    glm::vec3 rightVector = glm::cross(mViewDirection, mUpVector);
    mEye -= rightVector * speed;
}

void Camera::MoveRight(float speed)
{
    glm::vec3 rightVector = glm::cross(mViewDirection, mUpVector);
    mEye += rightVector * speed;
}

void Camera::Teleport(const glm::vec3& newPosition) {
    mEye = newPosition;
}