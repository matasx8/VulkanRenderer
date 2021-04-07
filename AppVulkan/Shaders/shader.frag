#version 450

layout(location = 0) out vec4 outColour; 
layout(location = 0) in vec3 fragCol;

void main()
{
	outColour = vec4(fragCol, 1.0);
}