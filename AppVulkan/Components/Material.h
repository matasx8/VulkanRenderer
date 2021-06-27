#pragma once
#include <string>
// Will define the material that a Model has.
// Should probably for each mesh, but it will do fine for each Model
class Material
{
public:
	Material();

	bool operator==(const Material& mat) const;

	~Material();

	// info variables
	const std::string vertexShader;
	const std::string fragmentShader;

};

