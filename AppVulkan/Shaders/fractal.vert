#version 450 //glsl 4.5
 
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 tex;

layout(location = 0) out vec3 fragTex;

	
void main()
{
	gl_Position = vec4(pos, 1.0);
}