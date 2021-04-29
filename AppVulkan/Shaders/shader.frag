#version 450

layout(location = 0) out vec4 outColour;
layout(location = 1) in vec2 fragTex;
layout(location = 0) in vec3 fragCol;
layout(set = 1, binding = 0) uniform sampler2D textureSampler;

void main()
{
	outColour = texture(textureSampler, fragTex);
}