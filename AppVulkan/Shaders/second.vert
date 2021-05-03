#version 450

vec2 positions[3] = vec2[](
	vec2(3.0, -1.0),
	vec2(-1.0, -1.0),
	vec2(-1.0, 3.0));//huge offscreen triangle
	
void main()
{//drawing a triangle with positions defined in shader. No need to pass anything here, because it's like the final blit or something
	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}