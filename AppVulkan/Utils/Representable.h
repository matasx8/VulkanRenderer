#pragma once
#include <string.h>
#include <string>

class Representable
{
public:
	Representable(){}
	virtual ~Representable(){}

	// Get the size of representation string + null termination symbol
	virtual size_t GetRepresentCstrLen() const = 0;
	// Will populate char array with representation.
	// Array must have allocated memory that will fit the representation
	// Use GetRepresentCstrLen() to find out the size needed
	virtual void RepresentCstr(char* const string, size_t size) const = 0;
};