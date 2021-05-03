#version 450

layout(input_attachment_index = 0, binding = 0) uniform subpassInput inputColour; //colour output from subpass 1
layout(input_attachment_index = 1, binding = 1)uniform subpassInput inputDepth; // depth output from subpass1

layout(location = 0) out vec4 colour;

void main()
{
	colour = subpassLoad(inputColour).rgba;
	colour.g = 0.0;
}