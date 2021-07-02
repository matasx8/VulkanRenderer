#pragma once
#include <string>
#include "Texture.h"
// Will define the material that a Model has.
// Should probably for each mesh, but it will do fine for each Model
class Material
{
public:
	Material();
	Material(const std::string vertexShader, const std::string fragmentShader);

	bool operator==(const Material& mat) const;

	// info variables
	std::string vertexShader;
	std::string fragmentShader;
	Texture texture;

};

