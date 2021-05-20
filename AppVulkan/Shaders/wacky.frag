#version 450

layout(location = 0) out vec4 outColour;
layout(location = 0) in vec3 colour;
layout(location = 1) in vec2 fragTex;
layout(location = 2) in float time;
//layout(location = 2) in vec2 resolution;//not sure about this

layout(set = 1, binding = 0) uniform sampler2D textureSampler;

void main()
{
	vec4 col = texture(textureSampler, fragTex);
	vec2 coord = (gl_FragCoord.xy / vec2(800, 600));
	
	col *= abs(sin(coord.x * 50.0 + cos(time + coord.y * 10.0 + sin(coord.x * 50.0 + time * 2.0))));
	col *= abs(cos(coord.x * 20.0 + sin(time + coord.y * 10.0 + cos(coord.x * 50.0 + time * 2.0))));
	col *= abs(sin(coord.x * 30.0 + cos(time + coord.y * 10.0 + sin(coord.x * 50.0 + time * 2.0))));
	col *= abs(cos(coord.x * 10.0 + sin(time + coord.y * 10.0 + cos(coord.x * 50.0 + time * 2.0))));
	outColour = vec4(col.x, col.y, col.z, col.w);
}