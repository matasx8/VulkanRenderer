#pragma once

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "NonCopyable.h"
#include "UniformProvider.h"
//TODO: add colour
//TODO: make directional
//TODO: change bightness
//TODO: add support for multiple lights
class Light : NonCopyable, UniformProvider
{
public:
	Light();
	Light(glm::vec4);//should later be used for direction

	void ProvideUniformData(void* dst);
	size_t ProvideUniformDataSize();

	//debug functions
	void debugInput(bool* keys, float deltaTime);
	void debugFollowCam(glm::vec4 newPos, glm::vec4 offset);
	void randomize();

	glm::vec4 m_Position;//the UI will need pointers to this so it's probably a good idea to leave them public
	glm::vec4 m_Colour;
	//float brightness;

private:

};

