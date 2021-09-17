#version 450 //glsl 4.5
 
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 tex;
layout (location = 3, binding = 1) in mat4 instance_transform;
 
layout(set = 0, binding = 0) uniform UboViewProjection{
	mat4 projection;
	mat4 view;
} uboViewProjection;

layout(set = 0, binding = 1) uniform UboLights
{
	vec4 position;
	vec4 colour;
} uboLights;
layout(set = 0, binding = 2) uniform Camera
{
vec4 pos;
} camera;
// not in use, left for reference
/*
layout(set = 0, binding = 1) uniform UboModel{
	mat4 model;
} uboModel;*/


layout(location = 0) out vec3 fragNorm;
layout(location = 1) out vec2 fragTex;
layout(location = 2) out vec3 fragPos;
layout(location = 3) out vec3 directionalLightSpacePos;
layout(location = 4) out vec3 directionalLightColour;
layout(location = 5) out vec3 camPos;
	
void main()
{
	gl_Position = uboViewProjection.projection * uboViewProjection.view * instance_transform; * vec4(pos, 1.0)
	
	fragPos = vec3(pushModel.model * vec4(pos, 1.0));
	fragTex = tex;
	fragNorm = norm;
	fragNorm = mat3(transpose(inverse(pushModel.model))) * norm;
	directionalLightSpacePos = uboLights.position.xyz;
	directionalLightColour = uboLights.colour.xyz;
	camPos = camera.pos.xyz;
}