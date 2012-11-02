//not completely compliant with OGL 4.2!!!
//need to define input value
//postion
//color
//normal and normal matrix

#version 420 core

#define POSITION		0
#define NORMAL			1
#define TEXCOORD    2

#define MATERIAL    3
#define LIGHT       4
#define TRANSFORM   5

layout(location = POSITION) in  vec3 position;
layout(location = NORMAL)in  vec3 normal;

layout(binding = TRANSFORM) uniform transform
{
	mat4 MVLeft;
	mat4 MVRight;
	mat4 PLeft;
	mat4 PRight;
} Transform;


out vec3 normalLeft;
out vec3 normalRight;
out vec4 color;



void main()
{
	gl_Position = vec4(position,1.0);
	color = vec4(0.8,0.8,0.0,1.0);
 
	mat4 NormalMatrixL = transpose(inverse(Transform.MVLeft));
	mat4 NormalMatrixR = transpose(inverse(Transform.MVRight));
	normalLeft = normalize((NormalMatrixL*vec4(normal,1.0)).xyz);
	normalRight = normalize((NormalMatrixR*vec4(normal,1.0)).xyz);
}