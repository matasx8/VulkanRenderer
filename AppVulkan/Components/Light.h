#pragma once

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "NonCopyable.h"
//TODO: add colour
//TODO: make directional
//TODO: change bightness
//TODO: add support for multiple lights
class Light : NonCopyable
{
public:
	Light();
	Light(glm::vec4);//should later be used for direction

	// I'd say these two are deprecated
	void getData(void* buffer);
	static size_t getDataSize();

	//debug functions
	void debugInput(bool* keys, float deltaTime);
	void debugFollowCam(glm::vec4 newPos, glm::vec4 offset);
	void randomize();

	glm::vec4 position;//the UI will need pointers to this so it's probably a good idea to leave them public
	glm::vec4 colour;
	//float brightness;

private:

};

