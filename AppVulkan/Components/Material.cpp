#include "Material.h"

Material::Material()
{

}

Material::Material(const std::string vertexShader, const std::string fragmentShader)
	: vertexShader(vertexShader), fragmentShader(fragmentShader)
{
}

bool Material::operator==(const Material& mat) const
{
	return vertexShader == mat.vertexShader;
}

Material::~Material()
{

}
