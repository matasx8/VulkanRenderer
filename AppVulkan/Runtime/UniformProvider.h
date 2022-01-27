#pragma once

class UniformProvider
{
public:

	// dst must have at least ProvideUniformDataSize() space allocated
	virtual void ProvideUniformData(void* dst) = 0;

	// returns transfer aligned data size
	virtual size_t ProvideUniformDataSize() = 0;

};