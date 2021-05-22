#include "Light.h"

Light::Light()
{
	this->position = glm::vec3(50.0f, 1000.0f, 0.0f);
	this->colour = glm::vec3(1.0f, 1.0f, 1.0f);
}

Light::Light(glm::vec3 position)
{
	this->position = position;
	this->colour = glm::vec3(1.0f, 1.0f, 1.0f);
}

Light::~Light()
{
}
