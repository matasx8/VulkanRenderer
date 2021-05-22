#pragma once
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
//TODO: add colour
//TODO: make directional
//TODO: change bightness
//TODO: add support for multiple lights
class Light
{
public:
	Light();
	Light(glm::vec3);//should later be used for direction

	void getData(void* buffer);
	static size_t getDataSize();

	~Light();
	//debug functions
	void debugInput(bool* keys, float deltaTime);

	glm::vec3 position;//the UI will need pointers to this so it's probably a good idea to leave them public
	glm::vec3 colour;
	//float brightness;

private:

};

