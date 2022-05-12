#version 450 //glsl 4.5
 
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 tex;

layout (location = 0) out vec4 color;
 
layout(set = 0, binding = 0) uniform UboViewProjection{
	mat4 PV;
} uboViewProjection;


layout(push_constant) uniform PushModel{
	mat4 model;
	vec4 color;
} pushModel;
	
void main()
{
	color = pushModel.color;
	gl_Position = uboViewProjection.PV * pushModel.model * vec4(pos, 1.0);
}