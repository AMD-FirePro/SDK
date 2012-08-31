#version 420 core

layout(location = 0) in vec3 Position0;
uniform mat4 matMVP;

void main()
{	
	gl_Position= matMVP * vec4(Position0, 1.0);
}
