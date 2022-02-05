#version 450 //glsl 4.5
 
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 tex;
 
layout(set = 0, binding = 0) uniform UboViewProjection{
	mat4 projection;
	mat4 view;
} uboViewProjection;


layout(push_constant) uniform PushModel{
	mat4 model;
} pushModel;
	
void main()
{
	gl_Position = uboViewProjection.projection * uboViewProjection.view * pushModel.model * vec4(pos, 1.0);
}