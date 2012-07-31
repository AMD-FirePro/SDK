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

    vec3 eyeVec = normalize(In.lightWorldPos-In.worldPosition);
	vec3 reflectedVec = reflect(-eyeVec, In.worldNormal);
	float pointLighting = max( dot( In.worldNormal, eyeVec ), 0.0);
	float directionalLighting1 = 0.5*max( dot( In.worldNormal, vec3(0,-2,0)), 0.0);
	float directionalLighting2 = 0.5*max( dot( In.worldNormal, vec3(0,2,0)), 0.0);
	vec3 specular = vec3(0.2*clamp((pow(dot(reflectedVec, eyeVec), 2.0f)), 0.0, 1.0));
	
    FragColor = vec4((0.05f*diffuseMat+(pointLighting+directionalLighting1+directionalLighting2)*diffuseMat+specular), 1.0f);
}
