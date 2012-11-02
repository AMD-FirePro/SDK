#version 420 core

uniform vec3 diffuseMat = vec3(0.4f, 0.25f, 0.2f);
uniform mat4 matView;

in block
{
	vec3 worldPosition;
	vec3 worldNormal;
	vec3 lightWorldPos;
} In;

layout(location = 0, index = 0) out vec4 FragColor;

void main()
{
	vec3 eyeVec = normalize(In.worldPosition-In.lightWorldPos);
	vec3 reflectedVec = reflect(eyeVec, In.worldNormal);
	float pointLighting = max( dot( In.worldNormal, eyeVec ), 0.0);
	float directionalLighting = max( dot( In.worldNormal, vec3(0,1,0)), 0.0);
	directionalLighting += max( dot( In.worldNormal, vec3(0,-1,0)), 0.0);
	float specular = 0.2*clamp((pow(dot(reflectedVec, eyeVec), 2.0f)), 0.0, 1.0);
	
	FragColor = vec4((0.05f*diffuseMat+(pointLighting+directionalLighting)*diffuseMat+specular), 1.0f);
}
