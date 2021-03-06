#include "Camera.h"

Camera::Camera()
	:msaaSamples(VK_SAMPLE_COUNT_1_BIT), position(glm::vec3(-12.0f, 33.0f, 25.0f)), worldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
	yaw(-60.0f), pitch(0.0f), front(glm::vec3(1.0f, -0.2f, -0.2f)), moveSpeed(80.0f), turnSpeed(0.5f)
{
}


Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw
	, float startPitch, float startMoveSpeed, float startTurnSpeed)
{
	position = startPosition;
	worldUp = startUp;
	yaw = startYaw;
	pitch = startPitch;
	front = glm::vec3(0.0f, 0.0f, -1.0f);

	moveSpeed = startMoveSpeed;
	turnSpeed = startTurnSpeed;

	msaaSamples = VK_SAMPLE_COUNT_1_BIT;

	update();
}

void Camera::keyControl(bool* keys, GLfloat deltaTime)
{
	GLfloat velocity = moveSpeed * deltaTime;
	if (keys[GLFW_KEY_W])
	{
		position += front * velocity;
	}

	if (keys[GLFW_KEY_S])
	{
		position -= front * velocity;
	}

	if (keys[GLFW_KEY_A])
	{
		position -= right * velocity;
	}

	if (keys[GLFW_KEY_D])
	{
		position += right * velocity;
	}
}

void Camera::mouseControl(GLfloat xChange, GLfloat yChange)
{

	xChange *= turnSpeed;
	yChange *= turnSpeed;

	yaw += xChange;
	pitch += yChange;

	if (pitch > 89.0f)
	{
		pitch = 89.0f;
	}

	if (pitch < -89.0f)
	{
		pitch = -89.0f;
	}

	update();
}

void Camera::setMSAA(VkSampleCountFlagBits msaaSamples)
{
	this->msaaSamples = msaaSamples > VK_SAMPLE_COUNT_8_BIT ? VK_SAMPLE_COUNT_8_BIT : msaaSamples;
}

glm::mat4 Camera::calculateViewMatrix()
{
	return glm::lookAt(position, position + front, up);
}

size_t Camera::GetRepresentCstrLen() const
{
	//TODO: find a way to calculate if float is for example 12 or 265465, now im just going to compensate
	return 100;
}

void Camera::RepresentCstr(char* const string, size_t size) const
{
	if (string)
	{
		char buff[100];
		sprintf_s(buff, "Position(x: %.1f; y: %.1f, z: %.1f) Front(x: %.1f; y: %.1f, z: %.1f)\n\0", position.x, position.y, position.z, front.x, front.y, front.z);
		strcpy_s(string, size, buff);
	}
}

glm::vec3& Camera::getCameraPosition()
{
	return position;
}


glm::vec3 Camera::getCameraDirection() const
{
	return glm::normalize(front);
}

void Camera::update()
{
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(front);//make unit vec

	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));

}