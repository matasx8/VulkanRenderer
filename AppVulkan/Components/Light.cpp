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
//get the data of the light and store it in the buffer
//must allocate the space before calling this
void Light::getData(void* pbuffer)
{
	//assume the buffer has space
	char* tptr = static_cast<char*>(pbuffer);
	memcpy(pbuffer, &position, sizeof(position));
	tptr += sizeof(glm::vec3) + 4;

	memcpy(tptr, &colour, sizeof(colour));
	/*debug
	float colr;
	memcpy(&colr, tptr, 4);
	tptr += sizeof(float);
	float colg;
	memcpy(&colg, tptr, 4);
	tptr += sizeof(float);
	float colb;
	memcpy(&colb, tptr, 4);
	*/
}

//get the size of the data used
//maybe try using the techinque
size_t Light::getDataSize()
{
	return sizeof(position) + sizeof(colour) + 4;//hacked
}

Light::~Light()
{
}

void Light::debugInput(bool* keys, float deltaTime)
{
	if (keys[GLFW_KEY_7])//R+
	{
		colour.r += 0.3 * deltaTime;
		colour.r = glm::clamp(colour.r, 0.0f, 1.0f);
	}
	else if (keys[GLFW_KEY_4])//R-
	{
		colour.r -= 0.3 * deltaTime;
		colour.r = glm::clamp(colour.r, 0.0f, 1.0f);
	}
	if (keys[GLFW_KEY_8])//G+
	{
		colour.g += 0.3 * deltaTime;
		colour.g = glm::clamp(colour.g, 0.0f, 1.0f);
	}
	else if (keys[GLFW_KEY_5])//G-
	{
		colour.g -= 0.3 * deltaTime;
		colour.g = glm::clamp(colour.g, 0.0f, 1.0f);
	}
	if (keys[GLFW_KEY_9])//B+
	{
		colour.b += 0.3 * deltaTime;
		colour.b = glm::clamp(colour.b, 0.0f, 1.0f);
	}
	else if (keys[GLFW_KEY_6])//B-
	{
		colour.b -= 0.3 * deltaTime;
		colour.b = glm::clamp(colour.b, 0.0f, 1.0f);
	}

}

void Light::debugFollowCam(glm::vec3 newPos, glm::vec3 offset)
{
	position = newPos + offset;
}
