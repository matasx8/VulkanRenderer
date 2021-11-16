#pragma once
#include "Representable.h"
#include "vulkan.h"
#include "NonCopyable.h"

//#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera : public Representable, NonCopyable
{
public:
	Camera();
	Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch
		, float startMoveSpeed, float startTurnSpeed);

	void keyControl(bool* keys, GLfloat deltaTime);
	void mouseControl(GLfloat xChange, GLfloat yChange);

	glm::vec3& getCameraPosition();
	glm::vec3 getCameraDirection() const;
	VkSampleCountFlagBits getMSAA() const { return m_MsaaSamples; }

	void LookAt(const glm::vec3 point);

	void setMSAA(VkSampleCountFlagBits msaaSamples);

	glm::mat4 calculateViewMatrix();

	//Representable
	size_t GetRepresentCstrLen() const;
	void RepresentCstr(char* const string, size_t size) const;

private:
	glm::vec3 m_Position;
	glm::vec3 m_Front;
	glm::vec3 m_Up;
	glm::vec3 m_Right;
	glm::vec3 m_WorldUp;

	float m_Yaw;
	float m_Pitch;

	float m_MoveSpeed;
	float m_TurnSpeed;

	VkSampleCountFlagBits m_MsaaSamples;

	void update();
};

