#pragma once
#include "Representable.h"
#include "vulkan.h"

//#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera : public Representable
{
public:
	Camera();
	Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch
		, float startMoveSpeed, float startTurnSpeed);

	void keyControl(bool* keys, GLfloat deltaTime);
	void mouseControl(GLfloat xChange, GLfloat yChange);

	glm::vec3& getCameraPosition();
	glm::vec3 getCameraDirection() const;
	VkSampleCountFlagBits getMSAA() const { return msaaSamples; }

	void setMSAA(VkSampleCountFlagBits msaaSamples);

	glm::mat4 calculateViewMatrix();

	//Representable
	size_t GetRepresentCstrLen() const;
	void RepresentCstr(char* const string, size_t size) const;

private:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	GLfloat yaw;
	GLfloat pitch;

	GLfloat moveSpeed;
	GLfloat turnSpeed;

	VkSampleCountFlagBits msaaSamples;

	void update();
};

