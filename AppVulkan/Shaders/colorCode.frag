#version 450
 
 layout(location = 0) in vec4 inColor;
layout(location = 0) out vec4 outColour;

layout(set = 1, binding = 0) uniform sampler2D textureSampler;

void main()
{
	outColour = inColor;
}