#pragma once
#include <glm/glm.hpp>
//TODO: add colour
//TODO: make directional
//TODO: change bightness
//TODO: add support for multiple lights
class Light
{
public:
	Light();
	Light(glm::vec3);//should later be used for direction

	~Light();

	glm::vec3 position;//the UI will need pointers to this so it's probably a good idea to leave them public
	glm::vec3 colour;
	//float brightness;

private:

};

