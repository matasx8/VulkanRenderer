#include "Light.h"
#include <random>

Light::Light()
	: m_Position(glm::vec4(50.0f, 1000.0f, 0.0f, 0.0f)), m_Colour(glm::vec4(1.0f, 1.0f, 1.0f, 0.0f))
{
}

Light::Light(glm::vec4 position)
	: m_Position(position), m_Colour(glm::vec4(1.0f, 1.0f, 1.0f, 0.0f))
{
}
void Light::ProvideUniformData(void* dst)
{
	//assume the buffer has space
	char* tptr = static_cast<char*>(dst);
	memcpy(dst, &m_Position, sizeof(m_Position));
	tptr += sizeof(glm::vec3) + 4;
	memcpy(tptr, &m_Colour, sizeof(m_Colour));
}
size_t Light::ProvideUniformDataSize()
{
	return sizeof(m_Position) + sizeof(m_Colour) + 4; // aligned
}

void Light::debugInput(bool* keys, float deltaTime)
{
	if (keys[GLFW_KEY_7])//R+
	{
		m_Colour.r += 0.3 * deltaTime;
		m_Colour.r = glm::clamp(m_Colour.r, 0.0f, 1.0f);
	}
	else if (keys[GLFW_KEY_4])//R-
	{
		m_Colour.r -= 0.3 * deltaTime;
		m_Colour.r = glm::clamp(m_Colour.r, 0.0f, 1.0f);
	}
	if (keys[GLFW_KEY_8])//G+
	{
		m_Colour.g += 0.3 * deltaTime;
		m_Colour.g = glm::clamp(m_Colour.g, 0.0f, 1.0f);
	}
	else if (keys[GLFW_KEY_5])//G-
	{
		m_Colour.g -= 0.3 * deltaTime;
		m_Colour.g = glm::clamp(m_Colour.g, 0.0f, 1.0f);
	}
	if (keys[GLFW_KEY_9])//B+
	{
		m_Colour.b += 0.3 * deltaTime;
		m_Colour.b = glm::clamp(m_Colour.b, 0.0f, 1.0f);
	}
	else if (keys[GLFW_KEY_6])//B-
	{
		m_Colour.b -= 0.3 * deltaTime;
		m_Colour.b = glm::clamp(m_Colour.b, 0.0f, 1.0f);
	}

}

void Light::debugFollowCam(glm::vec4 newPos, glm::vec4 offset)
{
	m_Position = newPos + offset;
}

void Light::randomize()
{
	m_Position = glm::vec4(std::rand(), std::rand(), std::rand(), std::rand());
	m_Colour = m_Position = glm::vec4(std::rand(), std::rand(), std::rand(), std::rand());
}
