#version 450

layout(location = 0) in vec3 fragClour;
layout(location = 0) out vec4 outColour; 

void main()
{
	outColour = vec4(fragClour, 1.0);
}