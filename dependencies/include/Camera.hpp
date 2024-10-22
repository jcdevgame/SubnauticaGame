#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

class Camera {
public:
	bool enabled = true;
	//Default constructor
	Camera();
	glm::vec3 mEye;
	glm::vec3 mViewDirection;

	//Ultimate view matrix to be produced + return
	glm::mat4 GetViewMatrix() const;

	void MouseLook(int mouseX, int mouseY);
	void MoveForward(float speed);
	void MoveBackward(float speed);
	void MoveLeft(float speed);
	void MoveRight(float speed);
	void Teleport(const glm::vec3& newPosition);

private:
	glm::vec3 mUpVector;
	glm::vec2 mOldMousePosition;
};

#endif