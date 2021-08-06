#pragma once
#include <string>
#include "Texture.h"
#include "Shader.h"
// Will define the material that a Model has.
// Should probably for each mesh, but it will do fine for each Model
class Material
{
public:
	Material();
	Material(ShaderCreateInfo& shaderInfo);

	const char* getVertexShader() const;
	const char* getFragmentShader() const;

	bool operator==(const Material& mat) const;

	// info variables
private:
	Shader m_Shader;
	std::vector<Texture> textures;

};