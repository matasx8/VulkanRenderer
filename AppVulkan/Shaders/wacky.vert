#version 450 //glsl 4.5

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 col;
layout (location = 2) in vec2 tex;

layout(set = 0, binding = 0) uniform UboViewProjection{
	mat4 projection;
	mat4 view;
} uboViewProjection;

// not in use, left for reference
layout(set = 0, binding = 1) uniform UboModel{
	mat4 model;
} uboModel;

layout(push_constant) uniform PushModel{
	mat4 model;
	float time;
} pushModel;

layout(location = 0) out vec3 fragCol;
layout(location = 1) out vec2 fragTex;
layout(location = 2) out float fragTime;

const float amplitude = 0.125;
const float frequency = 2;
const float PI = 3.14159;
	
void main()
{
	//float dist = length(pos) * 10.0;
    //float x = amplitude*sin(-PI*dist*frequency + fragTime);
   //gl_Position = MVP*vec4(vVertex.x, y, vVertex.z,1);
  
    //vec3 newpos = pos;
    //newpos += sin(pos.x * fragTime/* + cos(fragTime + pos.y * 10.0 + sin(pos.x * 50.0 + fragTime * 2.0))*/);
	//vec3 newpos = pos;
	gl_Position = uboViewProjection.projection * uboViewProjection.view * pushModel.model * vec4(pos, 1);//vec4(newpos.x, pos.y, pos.z, 1.0);
	
	fragCol = col;
	fragTex = tex;
	fragTime = pushModel.time;
}