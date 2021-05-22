#version 450

layout(location = 0) out vec4 outColour;

layout(location = 0) in vec3 fragNorm;
layout(location = 1) in vec2 fragTex;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 directionalLightSpacePos;
layout(location = 4) in vec3 directionalLightColour;
layout(location = 5) in vec3 viewPos;

layout(set = 1, binding = 0) uniform sampler2D textureSampler;

void main()
{
	float ambientStrength = 0.2;
	float specularStrength = 0.5;
	vec3 lightColor = directionalLightColour;
	vec3 lightPos = directionalLightSpacePos;
	vec3 lightDir = normalize(lightPos - fragPos);
	vec3 viewDir = normalize(viewPos - fragPos);//get a direction to light
	vec3 reflectDir = reflect(-lightDir, fragNorm);//we negate the lightDir because the reflect func expects
	// the first vetor to point from the light source towards the frag
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
	vec3 specular = specularStrength * spec * lightColor;
	
	vec3 ambient = ambientStrength * lightColor;
	
	vec3 norm = normalize(fragNorm);
	
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;
	
	vec4 objectColor = texture(textureSampler, fragTex);
	
	vec3 result = (ambient + diffuse + specular) * objectColor.xyz;
	outColour = vec4(result, objectColor.w);
}