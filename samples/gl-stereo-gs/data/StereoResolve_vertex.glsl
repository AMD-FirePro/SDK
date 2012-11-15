#version 420 core

#define POSITION		0
#define NORMAL			1
#define TEXCOORD		2


layout(location = POSITION) in  vec3 position;
layout(location = TEXCOORD) in  vec2 texCoord;

out vec3 vLeftCoords;
out vec3 vRightCoords;
void main()
{
	gl_Position = vec4(position,1.0);
	vLeftCoords = vec3(texCoord,-1.0);
	vRightCoords = vec3(texCoord,1.0); 
}