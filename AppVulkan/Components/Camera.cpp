#include "Camera.h"

Camera::Camera()
	: m_Position(glm::vec3(-12.0f, 33.0f, 25.0f)), m_Front(glm::vec3(1.0f, -0.2f, -0.2f)), m_WorldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
	m_Yaw(-60.0f), m_Pitch(0.0f), m_MoveSpeed(80.0f), m_TurnSpeed(0.5f), m_MsaaSamples(VK_SAMPLE_COUNT_1_BIT)
{
	update();
}


Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, 
	float startPitch, float startMoveSpeed, float startTurnSpeed)
	: m_Position(startPosition), m_Front(glm::vec3(1.0f, -0.2f, -0.2f)), m_WorldUp(startUp),
	m_Yaw(startYaw), m_Pitch(startPitch), m_MoveSpeed(startMoveSpeed), m_TurnSpeed(startTurnSpeed), m_MsaaSamples(VK_SAMPLE_COUNT_1_BIT)
{
	update();
}

void Camera::keyControl(bool* keys, GLfloat deltaTime)
{
	float velocity = m_MoveSpeed * deltaTime;
	if (keys[GLFW_KEY_W])
	{
		m_Position += m_Front * velocity;
	}

	if (keys[GLFW_KEY_S])
	{
		m_Position -= m_Front * velocity;
	}

	if (keys[GLFW_KEY_A])
	{
		m_Position -= m_Right * velocity;
	}

	if (keys[GLFW_KEY_D])
	{
		m_Position += m_Right * velocity;
	}
}

void Camera::mouseControl(GLfloat xChange, GLfloat yChange)
{

	xChange *= m_TurnSpeed;
	yChange *= m_TurnSpeed;

	m_Yaw += xChange;
	m_Pitch += yChange;

	if (m_Pitch > 89.0f)
	{
		m_Pitch = 89.0f;
	}

	if (m_Pitch < -89.0f)
	{
		m_Pitch = -89.0f;
	}

	update();
}

void Camera::LookAt(const glm::vec3 point)
{
	//m_Position = glm::lookAt(m_Position, point, m_Up);
}

void Camera::setMSAA(VkSampleCountFlagBits msaaSamples)
{
	this->m_MsaaSamples = msaaSamples > VK_SAMPLE_COUNT_8_BIT ? VK_SAMPLE_COUNT_8_BIT : msaaSamples;
}

glm::mat4 Camera::calculateViewMatrix()
{
	return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
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
		sprintf_s(buff, "Position(x: %.1f; y: %.1f, z: %.1f) Front(x: %.1f; y: %.1f, z: %.1f)\n\0", m_Position.x, m_Position.y, m_Position.z, m_Front.x, m_Front.y, m_Front.z);
		strcpy_s(string, size, buff);
	}
}

glm::vec3& Camera::getCameraPosition()
{
	return m_Position;
}


glm::vec3 Camera::getCameraDirection() const
{
	return glm::normalize(m_Front);
}

void Camera::update()
{
	m_Front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	m_Front.y = sin(glm::radians(m_Pitch));
	m_Front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	m_Front = glm::normalize(m_Front);//make unit vec

	m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
	m_Up = glm::normalize(glm::cross(m_Right, m_Front));

}